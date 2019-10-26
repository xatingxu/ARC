#include "wjk.hpp"

int main()
{
//    Robort r;
//    std::string str;
//    volatile bool quit=false;
//    while(!quit){
//        std::cout<<"Me#";
//       std::cin>>str;
//       std::string s=r.Talk(str);
//       std::cout<<"wjk$"<<s<<std::endl;
//    }
    wjk* w=new wjk();
    if(!w->LoadEtc()){
     return 1;
    }
    w->Run();
    return 0;
}
