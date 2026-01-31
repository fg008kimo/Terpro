/*
梅森旋转法
*/

#include<iostream>
#include<random>
#include<array>
#include<functional>
#include<algorithm>
#include<thread>
#include<mutex>
using namespace std;

#include "mt.h"
#include "../utils.h"

const int PLANTSEED_ROUND_MAX = 100000000;
static mt19937 engine;
static mutex engineMutex;

void MTHandle()
{
    random_device dropRd;
    mt19937 dropGen(dropRd());

    while(1)
    {
        {
            int num = 10 + dropGen() % 10; //每秒平均抽出至少10个随机数丢掉
            lock_guard<std::mutex> guard(engineMutex);
            engine.discard(num);
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void MTCreateHandleThread()
{
    thread t(MTHandle);
    t.detach();
}

int MTShouldPlantSeed()
{
    static int curRound = PLANTSEED_ROUND_MAX;

    curRound++;
    if(curRound > PLANTSEED_ROUND_MAX)
    {
        curRound = 0;
        return 1;
    }

    return 0;
}

void MTPlantSeed()
{
    random_device rd;
    array<unsigned int, mt19937::state_size> seed_data;
    generate_n(seed_data.begin(), seed_data.size(), ref(rd));
    seed_seq seq(begin(seed_data), end(seed_data));
    lock_guard<std::mutex> guard(engineMutex);
    engine.seed(seq);
}

void MTPreHanle()
{
    if(MTShouldPlantSeed())
        MTPlantSeed();
}

/////////////////////////////////【对外接口】////////////////////////////////////////

extern "C" {
void MTInit()
{
    MTCreateHandleThread();
}

double MTGetRandom()
{
    uniform_real_distribution<> dist(0, 1);
    MTPreHanle();
    lock_guard<std::mutex> guard(engineMutex);
    return dist(engine);
}

uint32_t MTGetRand(uint32_t minValue,uint32_t maxValue)
{
    uniform_int_distribution<uint32_t> dist(minValue, maxValue-1);
    MTPreHanle();
    lock_guard<std::mutex> guard(engineMutex);
    return dist(engine);
}
}

