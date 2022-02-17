#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <conio.h>
#include <map>
#include <bits/stdc++.h>

using namespace std;

#define INPUT_PATH "./input/input2.txt"
#define OUTPUT_PATH1 "./output/output1.txt"
#define OUTPUT_PATH2 "./output/output2.txt"
#define ABS_DEADLINE 0
#define REMAIN_EXEC 1
#define ABS_START 2
#define PID 3

int count = 0;

class Process
{
public:
    int pid;
    int start;
    int execution;
    int deadline;
    int period;
    Process(int pid, int executionTime, int deadlineTime, int periodTime)
    {
        this->pid = pid;
        this->start = 0;
        this->execution = executionTime;
        this->deadline = deadlineTime;
        this->period = periodTime;
    }

private:
    void printDebug()
    {
    }
};

/*  Read input for each process. It must be ensured that input format is correct
    numberOfProcesses
    start1 execution1 deadline1 period1
    start2 execution2 deadline2 period2
    ...
*/
void readInputFile(vector<Process> &data)
{
    ifstream fin(INPUT_PATH);
    int n;
    fin >> n;
    cout << n << endl;
    int pid, executionTime, deadlineTime, periodTime;

    for (int i = 0; i < n; ++i)
    {
        fin >> pid >> executionTime >> deadlineTime >> periodTime;
        cout << pid << " " << executionTime << " " << deadlineTime << " " << periodTime << endl;

        /* Validate input */
        if (executionTime > deadlineTime)
        {
            cout << "[ERROR] invalid input - executionTime > deadlineTime, can't schedule" << endl;
            exit(0);
        }

        if (executionTime > periodTime)
        {
            cout << "[ERROR] invalid input - executionTime > periodTime, unlimited tasks" << endl;
            exit(0);
        }

        //TO-DO: check if there are same pid

        Process p(pid, executionTime, deadlineTime, periodTime);
        data.push_back(p);
    }
    fin.close();
}

void checkCpuUtilization(vector<Process> &data)
{
    double cu = 0;
    for (int i = 0; i < data.size(); ++i)
        cu += (double)data[i].execution / data[i].period;
    if (cu > 1)
    {
        cout << "[ERROR] invalid input - CPU utilization > 1, unlimited tasks" << endl;
        exit(0);
    }
}

int gcd(int a, int b)
{
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

/* Function to return LCM of two numbers */
int lcm(int a, int b)
{
    return (a / gcd(a, b)) * b;
}

int main()
{
    /* read input */
    vector<Process> data;
    readInputFile(data);
    int n = data.size();


    
    /* output files */
    ofstream out1(OUTPUT_PATH1, std::ofstream::out | std::ofstream::trunc);
    ofstream out2(OUTPUT_PATH2, std::ofstream::out | std::ofstream::trunc);

    int hyperPeriod = 1;
    for (int i = 0; i < n; ++i)
        hyperPeriod = lcm(hyperPeriod, data[i].period);

    priority_queue<vector<int>, vector<vector<int> >, greater<vector<int> > > taskQueue;
    vector<int> processData(4); 

    /* Loop for 1 hyper period */
    int totalProcesses = 0;
    int completedProcesses = 0;
    int missedDeadline = 0;
    map<int, int> fullExecTime;
    map<int, vector<float> > avrWaitingTime;
    for (int i = 0; i < n; ++i) {
        fullExecTime.insert({data[i].pid, data[i].execution});
        avrWaitingTime.insert({data[i].pid, vector<float>(2, 0)});
    }
    

    int timeCounter = 0;
    while (timeCounter <= hyperPeriod)
    {
        if (!taskQueue.empty()) {
            if (taskQueue.top()[REMAIN_EXEC] == 0) {
                /* finish log */
                vector<int> cur = taskQueue.top();
                completedProcesses++;
                if (cur[ABS_DEADLINE] < timeCounter) missedDeadline++;
                avrWaitingTime[cur[PID]][0] =  (avrWaitingTime[cur[PID]][0]*avrWaitingTime[cur[PID]][1] + timeCounter - cur[ABS_START])/(avrWaitingTime[cur[PID]][1] + 1);

                out1 << "[FINISH]Process " << taskQueue.top()[PID] << " finishes execution at timestamp " << timeCounter << ". Start: " << taskQueue.top()[ABS_START] << endl;
                taskQueue.pop();
                if (!taskQueue.empty()) {
                    /* start or resume another thread */
                    vector<int> v = taskQueue.top();
                    if (v[REMAIN_EXEC] == fullExecTime[v[PID]]) {
                        out1 << "[START]Process " << taskQueue.top()[PID] << " starts execution at timestamp " << timeCounter << endl;
                    }
                    else {
                        out1 << "[RESUME]Process " << taskQueue.top()[PID] << " resumes execution at timestamp " << timeCounter << endl;
                    }
                }
            }
        }

        /* At the begin of timeframe, record status of all pending tasks to out2 */
        out2 << "[Time:" << timeCounter << "][Total processes: " << totalProcesses << "][Completed processes: " << completedProcesses << "][Missed deadline times: " << missedDeadline << "]" << endl;
        out2 << "Average waiting time: ";
        for (int i = 0; i < n; ++i) {
            out2 << "[Process" << data[i].pid << "]:" << avrWaitingTime[data[i].pid][0] << "s; ";
        }
        out2 << endl;


        
        bool idleState = taskQueue.empty();
        vector<int> prev;
        if (!idleState) {
            prev = taskQueue.top();
        }

        /* Add new task(s) to task queue */
        bool pushed = false;
        for (int i = 0; i < n; ++i) {
            if (timeCounter%data[i].period == 0) {
                processData[ABS_DEADLINE] = data[i].deadline + timeCounter;
                processData[REMAIN_EXEC] = data[i].execution;
                processData[ABS_START] = timeCounter;
                processData[PID] = data[i].pid;

                /* join log */
                if (timeCounter == data[i].start) {
                    out1 << "[JOIN]Process " << data[i].pid << " has joined at timestamp " << timeCounter << ". Processing time: " << data[i].execution << "; Deadline: " << data[i].deadline << "; Perioid: " << data[i].period << ";" << endl;
                }
                taskQueue.push(processData);
                pushed = true;
                totalProcesses++;
            }

            if (i == n-1 && pushed) {
                if (idleState) {
                    out1 << "[START]Process " << taskQueue.top()[PID] << " starts execution at timestamp " << timeCounter << endl;
                }
                else {
                    if (taskQueue.top()[PID] != prev[PID]) {
                        out1 << "[PREEMPTED]Process " << prev[PID] << " is preempted by another process at " << timeCounter << ". Remaining processing time: " << prev[REMAIN_EXEC] << endl;
                        out1 << "[START]Process " << taskQueue.top()[PID] << " starts execution at timestamp " << timeCounter << endl;
                    }
                }
            }
        }

        /* Execute the highest priority task from timeCounter to timeCounter+1 */
        if (!taskQueue.empty()) {
            vector<int> v = taskQueue.top(); taskQueue.pop();
            v[REMAIN_EXEC]--;
            taskQueue.push(v);
        }
        else out1 << "CPU is idle from timestamp " << timeCounter << " to " << timeCounter+1 << endl;
        timeCounter++;
    }

    out1.close();
    out2.close();
    cout << "The program runs successfully" << endl;
    getch();
    return 0;
}
