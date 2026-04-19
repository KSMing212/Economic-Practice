#include <iostream>
#include <cmath>
using namespace std;

int main(){
    double a,b,c,d,Qd,Qs,P;
    double n;
    cout << "请按顺序输入需求函数的截距与斜率（绝对值）和供给函数的截距和斜率" << endl;
    cin >> a >> b >> c >>d ;
    cout <<"请输入跨度"<<endl;
    cin >> n;
    for(P=0;P<=1000000;P+=n){
        Qd=a-b*P;
        Qs=c+d*P;
        if(Qd-Qs<=0){
            double Q1=Qd-Qs;
            double P1=P-n;
            double Qd1=a-b*P1,Qs1=c+d*P1;
            double Q2=Qd1-Qs1;
            if(fabs(Q1)<fabs(Q2)){
            cout <<"均衡价格大约为"<<P<<endl;
            cout <<"此时需求为"<<Qd<<",此时供给为"<<Qs<<endl;
            break;}
            if(fabs(Q1)>fabs(Q2)){
            cout <<"均衡价格大约为"<<P1<<endl;
            cout <<"此时需求为"<<Qd1<<",此时供给为"<<Qs1<<endl;
            break;
            }
            else{
                P=(P+P1)/2;
                cout <<"均衡价格大约为"<<P<<endl;
                cout <<"此时需求为"<<Qd<<",此时供给为"<<Qs<<endl;
                break;}
        }
    }
    return 0;
    }
