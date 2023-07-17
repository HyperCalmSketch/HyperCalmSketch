#include <bits/stdc++.h>
using namespace std;
const int TOPK_THRESHOLD = 100; 
const double BATCH_TIME_THRESHOLD = 0.727 / 5e4;
const double UNIT_TIME = BATCH_TIME_THRESHOLD * 10;
unsigned int rll(){
    return ((rand()<<16)|rand())+1;
}
vector<pair<float,int> >out;
int main(){
    freopen("synthesis.dat","w",stdout);
    unsigned int n=200,m=50000;
    unsigned int mx=(1ll<<32)-1;
    for(int i=1;i<=n;i++){
        float first=100.0*UNIT_TIME*rll()/mx;
        float period;
        if(n<=200){
            period=10*UNIT_TIME;
        }else{
            period=(rll()%200+5)*UNIT_TIME;
        }
        while(first<m*UNIT_TIME){
            for(int k=rll()%1+1;k;k--){
                out.push_back(make_pair(first+k*BATCH_TIME_THRESHOLD/2,i));
            }
            first+=period;
        }
    }
    for(int i=200;i<=m;i++)
    for(int j=1;j<=100;j++){
        if(rand()%2==0)n++;
        out.push_back(make_pair(i*UNIT_TIME,n));
    }
    sort(out.begin(),out.end());
    for(auto x:out){
        printf("%d %.10lf\n",x.second,x.first);
    }
    return 0;
}
