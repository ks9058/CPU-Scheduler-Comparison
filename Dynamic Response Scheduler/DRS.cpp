#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

const int Q1_MAX_EXEC = 40;  // Q1 최대 연속 실행 시간

struct Process {
    int type;
    int pid;
    int priority;
    int computing_time;
    int arrival_time;
    int remaining_time;
    int start_time = -1;
    int finish_time = -1;
    int executed_time_in_current_queue = 0;
    int current_queue = 1;

    Process() {}
    Process(int t, int p, int pri, int comp, int at)
        : type(t), pid(p), priority(pri), computing_time(comp),
        arrival_time(at), remaining_time(comp) {}
};

vector<Process> input_processes;
vector<Process> finished_processes;

vector<Process> Q1;  // SRTF
vector<Process> Q2;  // HRRN

int current_time = 0;

// 도착한 프로세스를 Q1에 추가
void check_arrivals() {
    for (auto it = input_processes.begin(); it != input_processes.end();) {
        if (it->arrival_time <= current_time) {
            it->current_queue = 1;
            it->executed_time_in_current_queue = 0;
            Q1.push_back(*it);
            it = input_processes.erase(it);
        }
        else {
            ++it;
        }
    }
}

// SRTF 실행 (1단위 시간)
bool execute_srtf_one_unit() {
    if (Q1.empty()) return false;

    auto it = min_element(Q1.begin(), Q1.end(), [](const Process& a, const Process& b) {
        return a.remaining_time < b.remaining_time;
        });

    Process p = *it;
    Q1.erase(it);

    if (p.start_time == -1) p.start_time = current_time;

    p.remaining_time--;
    p.executed_time_in_current_queue++;
    current_time++;

    check_arrivals();

    if (p.remaining_time == 0) {
        p.finish_time = current_time;
        finished_processes.push_back(p);
    }
    else {
        if (p.executed_time_in_current_queue >= Q1_MAX_EXEC) {
            p.executed_time_in_current_queue = 0;
            p.current_queue = 2;
            Q2.push_back(p);
        }
        else {
            Q1.push_back(p);
        }
    }

    return true;
}

// HRRN 실행 (전체 대기시간 기준)
bool execute_hrrn() {
    if (Q2.empty()) return false;

    auto it = max_element(Q2.begin(), Q2.end(), [](const Process& a, const Process& b) {
        double waitA = current_time - a.arrival_time;
        double waitB = current_time - b.arrival_time;
        double rra = (waitA + a.remaining_time) / (double)a.remaining_time;
        double rrb = (waitB + b.remaining_time) / (double)b.remaining_time;
        return rra < rrb;
        });

    
    Process p = *it;
    Q2.erase(it);

    if (p.start_time == -1) p.start_time = current_time;

    current_time += p.remaining_time;
    p.remaining_time = 0;
    p.finish_time = current_time;

    check_arrivals();
    finished_processes.push_back(p);

    return true;
}

// 스케줄링 사이클 실행
bool schedule_one_cycle() {
    check_arrivals();

    if (!Q1.empty()) return execute_srtf_one_unit();
    else if (!Q2.empty()) return execute_hrrn();
    else if (!input_processes.empty()) {
        current_time++;
        check_arrivals();
        return true;
    }

    return false;
}

int main() {
    ifstream infile("input.txt");
    if (!infile.is_open()) {
        cerr << "Error: Cannot open input.txt\n";
        return 1;
    }

    int type, pid, priority, computing_time;
    int current_input_time = 0;  // 시뮬레이션용 도착 시간

    while (infile >> type >> pid >> priority >> computing_time) {
        if (type == -1) break;

        if (type == 1) {
            current_input_time += 20;  // 시간 퀀텀 경과
            continue;
        }

        if (type == 0) {
            Process p(type, pid, priority, computing_time, current_input_time);
            input_processes.push_back(p);
        }
    }

    infile.close();

    while (schedule_one_cycle()) {}

    // 출력
    cout << "----------------------------------------------------------\n";
    cout << left << setw(12) << "Process_id" << setw(12) << "priority" << setw(18)
        << "computing_time" << setw(18) << "turn_around_time" <<"\n";
    cout << "----------------------------------------------------------\n";

    for (auto& p : finished_processes) {
        int tat = p.finish_time - p.arrival_time;
        cout << left << setw(12) << p.pid << setw(12) << int(p.priority)
            << setw(18) << p.computing_time << setw(18) << tat <<"\n";
    }

    cout << "----------------------------------------------------------\n";
    double total_normalized_tat = 0.0;
    for (auto& p : finished_processes) {
        int tat = p.finish_time - p.arrival_time;
        total_normalized_tat += (double)tat / p.computing_time;
    }

    double normalized_avg_tat = total_normalized_tat / finished_processes.size();
    cout << "Normalized Average Turnaround Time: " << fixed << setprecision(3)
        << normalized_avg_tat << "\n";

    return 0;
}
