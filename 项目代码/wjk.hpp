#pragma once
#include <unistd.h>
#include <cstdio>
#include <map>
#include <pthread.h>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <json/json.h>
#include <memory>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <base/http.h>
#include <stdlib.h>
#include <speech.h>

#define TTS_PATH "temp_file/asr.wav"
#define LOG "log.txt"
#define ASR_PATH "temp_file/asr.wav"
//#define TTS_PATH "temp_file/tts.mp3"
#define CMD_ETC "command.etc"


class until{
    private:
        static pthread_t id;
    public:
        static bool Exec(std::string command,bool is_print)
        {
            if(!is_print){
                command+=">/dev/null 2>&1";
            }
            FILE *fp=popen(command.c_str(),"r");
            if(nullptr==fp){
                std::cerr<<"popen exec \'"<<command<<"\'Error"<<std::endl;
                return false;
            }
            if(is_print){
            char ch;
            while(fread(&ch,1,1,fp)>0)
            {
                fwrite(&ch,1,1,stdout);
            }
            }
            pclose(fp);
            return true;
        }

    static void *ThreadRun(void *arg)
    {
        pthread_detach(pthread_self());
        const char* tips=(char*)arg;
        int i=0;
        char bar[103]={0};
        const char* label="|/-\\";
        for(; i<=50; i++){
          printf("%s[%-51s][%d%%][%c]\r",tips,bar,i*2,label[i%4]);
          fflush(stdout);
          bar[i]='=';
          bar[i+1]='>';
          usleep(49000*2);
        }
        printf("\n");
    }
    static void PrintStart(std::string tips)
    {
        pthread_create(&id,NULL,ThreadRun,(void*)tips.c_str());
    }
    static void PrintEnd()
    {
        pthread_cancel(id);
    }
};
pthread_t until::id;


class Robort{
    private:
      std::string url;
      std::string api_key;
      std::string user_id;
      aip::HttpClient client;
    private:
    bool IsCodeLegal(int code)
    {
        bool result=false;
        switch(code){
            case 5000:
               break;
            case 10004:
               result=true;
               break;
               default:
               result = true;
               break;
        }
        return result;
    }

       std::string MessageToJson(std::string &message)
        {
            Json::Value root;
            Json::StreamWriterBuilder wb;
            std::ostringstream ss;

            Json::Value _item;
            _item["text"]=message;

            Json::Value item;
            item["inputText"]=_item;


            root["reqType"]=0;//text
            root["perception"]=item;

            item.clear();
            item["apiKey"]=api_key;
            item["userId"]=user_id;
            root["userInfo"]=item;

            std::unique_ptr<Json::StreamWriter> sw(wb.newStreamWriter());
            sw->write(root,&ss);
            std::string json_string=ss.str();
           // std::cout<<"debug: "<<json_string<<std::endl;
            return json_string;
        }
        std::string RequestTL(std::string &request_json)
        {
            std::string response;
            int status_code= client.post(url,nullptr,request_json,nullptr,&response);
            if(status_code!=CURLcode::CURLE_OK){
                std::cerr<<"post error"<<std::endl;
                return "";
        }
        return response;
        }
        std::string JsonToEchoMessage(std::string &str)
        {
           // std::cout<<"JsonEchoMessage: "<<str<<std::endl;
           JSONCPP_STRING errs;
           Json::Value root;
           Json::CharReaderBuilder rb;
           std::unique_ptr<Json::CharReader> const cr(rb.newCharReader());
           bool res=cr->parse(str.data(),str.data()+str.size(),&root,&errs);
           if(!res||!errs.empty()){
               std::cerr<<"parse error!"<<std::endl;
            return "";
        }
        int code=root["intent"]["code"].asInt();
        if(!IsCodeLegal(code))
        {
            std::cerr<<"reponse code error"<<std::endl;
            return "";

     }
     Json::Value item=root["results"][0];
     std::string msg= item["values"]["text"].asString();
     return msg;
     }
    public:
      Robort(std::string id="1")
      {
          this->url="http://openapi.tuling123.com/openapi/api/v2";
          this->api_key="16c8e86b57cd4f94bfe18a6041dd0abf";
          this->user_id=id;

      }
      std::string Talk(std::string message)
      {
         std::string json = MessageToJson(message);
         std::string response = RequestTL(json);
         std::string echo_message = JsonToEchoMessage(response);
         return echo_message;

      }
      ~Robort()
      {

      }

};

class SpeechRec{
   private:
   std::string app_id;
   std::string api_key;
   std::string secret_key;
   aip::Speech *client;
   private:
     bool IsCodeLegal(int code)
    {
        bool result=false;
        switch(code){
            case 0:
            result=true;
               break;
              break;
               default:
               break;
        }
        return result;
}     
   public:
     SpeechRec(){
         app_id="16892972";
         api_key="c2SBdH5XHBZi7cFkt8OZCe2u";
         secret_key="VrvXSpE1pMdvSXSp7wfb0502T08suGxF";
        client=new aip::Speech(app_id,api_key,secret_key);

     }

    bool ASR(std::string path,std::string &out)
    {
     std::map<std::string,std::string> options;
     options["den_pid"]="1536";
     std::string file_content;
     aip::get_file_content(ASR_PATH,&file_content);
     Json::Value result= client->recognize(file_content,"wav",16000,options);
    //std::cout<<"debug: "<<result.toStyledString()<<std::endl;
    int code=result["err_no"].asInt();
    if(!IsCodeLegal(code)){
        std::cout<<"recognize error"<<std::endl;
        return false;
    }
    out=result["result"][0].asString();
     return true;
    }

    bool TTS(std::string message){
      bool ret;
      std::ofstream ofile;
      std::string ret_file;
      std::map<std::string,std::string> options;
      options["spd"]="5";//语速0-15
      options["pit"]="7";//语调//0-15
      options["vol"]="15";//风格0-15c
      options["per"]="110";//1 0 3 4 110 111 103 106 5
      options["aue"]="6";
      ofile.open(TTS_PATH,std::ios::out|std::ios::binary);

      Json::Value result =client->text2audio(message,options,ret_file);
      if(!ret_file.empty()){
          ofile<<ret_file;
          ofile.close();
          ret=true;
      }
      else{
          std::cerr<<result.toStyledString()<<std::endl;
          ret=false;
    }
    ofile.close();
    return ret;
}
     ~SpeechRec(){
     }

    
};

class wjk{
    private:
       Robort rt;
       SpeechRec sr;
       std::unordered_map<std::string,std::string> commands;
       private:
       bool Record()
       {
           //std::cout<<"debug: "<<"Record..."<<std::endl;
           until::PrintStart("录音中：");
           std::string command="arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
           command += ASR_PATH;
           bool ret=until::Exec(command,false);
           //std::cout<<"debug:Recond ... done"<<std::endl;
           until::PrintEnd();
           return ret;
       }

       bool Play(){
         std::string command="cvlc --play-and-exit ";
         command += TTS_PATH;
         return until::Exec(command,false);
}
    public:
      wjk()
      {
          

      };

      bool LoadEtc(){
      std::ifstream in(CMD_ETC);
      if(!in.is_open()){
          std::cerr<<"open error"<<std::endl;
          return false;
      }
      std::string sep = ":";
      char line[256];
      while(in.getline(line,sizeof(line))){
          std::string str=line;
          std::size_t pos=str.find(sep);
          if(std::string::npos==pos){
              std::cerr<<"not find:"<<std::endl;
              continue;
      }
      std::string k=str.substr(0,pos);
      std::string v=str.substr(pos+sep.size());
      k+="。";
      commands.insert(std::make_pair(k,v));
      //commands.insert({k,v});
      }
      std::cerr<<"Load command etc done ...success"<<std::endl;
      in.close();
      return true;
    }
      bool IsCommand(std::string message,std::string &cmd)
      {
          auto iter=commands.find(message);
          //std::cout<<message<<std::endl;
          if(iter==commands.end()){
              return false;
              //std::cout<<"false"<<std::endl;
          }
          cmd=iter->second;
          //std::cout<<"true"<<std::endl;
          return true;
      }
      void Run()
      {
#ifdef _LOG_
          int fd=open(LOG,O_WRONLY|O_CREAT,0644);
          dup2(fd,2);

#endif
          volatile bool quit=false;
          while(!quit){
              if(this->Record()){
                   std::string message;
                   if(sr.ASR(ASR_PATH,message))
                   {
                     //1.command
                     std::string cmd="";
                     if(IsCommand(message,cmd)){
                     std::cout<<"[wjk@localhost]$"<<cmd<<std::endl;
                     until::Exec(cmd,true);
                     continue;

       }
       std::cout<<"我# "<<message<<std::endl;
       if(message=="你可以走了。"){
         std::string quit_message="好吧，那我走了，不要太想我哦！";
         std::cout<<"偶像wjk# "<<quit_message<<std::endl;
         if(sr.TTS(quit_message)){
             this->Play();
         }
         exit(0);
       }
       //2.tuling
      //std::cout<<"我# "<<message<<std::endl;
      std::string echo = rt.Talk(message);
      std::cout<<"偶像wjk# "<<echo<<std::endl;
       if(sr.TTS(echo)){
        this->Play();
       }      
      }
     else{
       std::cerr<<"Recognize error..."<<std::endl;

               }
              }
              else{
                  std::cerr<<"Record error..."<<std::endl;
           } 
                 sleep(2);
          }
#ifdef _LOG_//如果有LOG就往LOG里打
          close(fd);
#endif
      }
      ~wjk()
      {

    }
};
