#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Process {
    int arrival_time;
    int process_id;
    int priority;
    int computing_time;
    int remaining_time;
    int final_priority;
    string final_queue;
    int turnaround_time;
    bool is_real_time;
    int entry_order;

    Process(int arrival, int pid, int prio, int ctime, int order)
        : arrival_time(arrival), process_id(pid), priority(prio), computing_time(ctime),
        remaining_time(ctime), final_priority(prio), final_queue(""), turnaround_time(0),
        is_real_time(prio < 0), entry_order(order) {}
};

struct Compare {
    bool operator()(const Process* a, const Process* b) const {
        if (a->priority == b->priority)
            return a->entry_order > b->entry_order;
        return a->priority > b->priority;
    }
};

const int TIME_QUANTUM = 20;

void read_input(vector<Process*>& pending);
void insert_arrived_processes(vector<Process*>& pending, size_t& idx, int current_time,
    queue<Process*>& rtq,
    priority_queue<Process*, vector<Process*>, Compare>& Q1,
    priority_queue<Process*, vector<Process*>, Compare>& Q2,
    priority_queue<Process*, vector<Process*>, Compare>& Q3,
    queue<Process*>& Q4);

bool handle_real_time_queue(queue<Process*>& rtq, vector<Process*>& finished, int& time);
Process* select_next_process(priority_queue<Process*, vector<Process*>, Compare>& Q1,
    priority_queue<Process*, vector<Process*>, Compare>& Q2,
    priority_queue<Process*, vector<Process*>, Compare>& Q3,
    queue<Process*>& Q4,
    string& queue_id);
void execute_process(Process* current, int& time, vector<Process*>& finished,
    const string& queue_id,
    priority_queue<Process*, vector<Process*>, Compare>& Q1,
    priority_queue<Process*, vector<Process*>, Compare>& Q2,
    priority_queue<Process*, vector<Process*>, Compare>& Q3,
    queue<Process*>& Q4);

void schedule_processes(vector<Process*>& pending, vector<Process*>& finished);
void print_results(const vector<Process*>& finished);
void cleanup(vector<Process*>& finished);

int main() {
    vector<Process*> pending;
    vector<Process*> finished;

    read_input(pending);
    schedule_processes(pending, finished);
    print_results(finished);
    cleanup(finished);
    return 0;
}

void read_input(vector<Process*>& pending) {
    ifstream infile("input.txt");
    string line;
    int current_time = 0;
    int entry_order = 0;

    while (getline(infile, line)) {
        if (line == "-1") break;
        stringstream ss(line);
        int type, pid, prio, ctime;
        ss >> type >> pid >> prio >> ctime;

        if (type == 0) {
            Process* p = new Process(current_time, pid, prio, ctime, entry_order++);
            pending.push_back(p);
        }
        else {
            current_time += TIME_QUANTUM;
        }
    }
}

void insert_arrived_processes(vector<Process*>& pending, size_t& idx, int current_time,
    queue<Process*>& rtq,
    priority_queue<Process*, vector<Process*>, Compare>& Q1,
    priority_queue<Process*, vector<Process*>, Compare>& Q2,
    priority_queue<Process*, vector<Process*>, Compare>& Q3,
    queue<Process*>& Q4) {
    while (idx < pending.size() && pending[idx]->arrival_time <= current_time) {
        Process* p = pending[idx++];
        if (p->is_real_time) rtq.push(p);
        else if (p->priority <= 10) Q1.push(p);
        else if (p->priority <= 20) Q2.push(p);
        else if (p->priority <= 30) Q3.push(p);
        else Q4.push(p);
    }
}

bool handle_real_time_queue(queue<Process*>& rtq, vector<Process*>& finished, int& time) {
    if (!rtq.empty()) {
        Process* rt = rtq.front(); rtq.pop();
        time += rt->remaining_time;
        rt->final_queue = "real_time";
        rt->turnaround_time = time - rt->arrival_time;
        finished.push_back(rt);
        return true;
    }
    return false;
}

Process* select_next_process(priority_queue<Process*, vector<Process*>, Compare>& Q1,
    priority_queue<Process*, vector<Process*>, Compare>& Q2,
    priority_queue<Process*, vector<Process*>, Compare>& Q3,
    queue<Process*>& Q4,
    string& queue_id) {
    if (!Q1.empty()) { queue_id = "Q1"; Process* p = Q1.top(); Q1.pop(); return p; }
    if (!Q2.empty()) { queue_id = "Q2"; Process* p = Q2.top(); Q2.pop(); return p; }
    if (!Q3.empty()) { queue_id = "Q3"; Process* p = Q3.top(); Q3.pop(); return p; }
    if (!Q4.empty()) { queue_id = "Q4"; Process* p = Q4.front(); Q4.pop(); return p; }
    return nullptr;
}

void execute_process(Process* current, int& time, vector<Process*>& finished,
    const string& queue_id,
    priority_queue<Process*, vector<Process*>, Compare>& Q1,
    priority_queue<Process*, vector<Process*>, Compare>& Q2,
    priority_queue<Process*, vector<Process*>, Compare>& Q3,
    queue<Process*>& Q4) {
    int exec_time = min(current->remaining_time, TIME_QUANTUM);
    time += exec_time;
    current->remaining_time -= exec_time;

    if (current->remaining_time == 0) {
        current->final_queue = queue_id;
        current->turnaround_time = time - current->arrival_time;
        finished.push_back(current);
    }
    else {
        current->priority = min(current->priority + 10, 31);
        if (current->priority <= 10) Q1.push(current);
        else if (current->priority <= 20) Q2.push(current);
        else if (current->priority <= 30) Q3.push(current);
        else Q4.push(current);
    }
}

void schedule_processes(vector<Process*>& pending, vector<Process*>& finished) {
    queue<Process*> rtq;
    priority_queue<Process*, vector<Process*>, Compare> Q1, Q2, Q3;
    queue<Process*> Q4;

    int time = 0;
    size_t idx = 0;

    while (finished.size() < pending.size()) {
        insert_arrived_processes(pending, idx, time, rtq, Q1, Q2, Q3, Q4);

        if (handle_real_time_queue(rtq, finished, time)) continue;

        string queue_id;
        Process* current = select_next_process(Q1, Q2, Q3, Q4, queue_id);

        if (current) {
            execute_process(current, time, finished, queue_id, Q1, Q2, Q3, Q4);
            insert_arrived_processes(pending, idx, time, rtq, Q1, Q2, Q3, Q4);
        }
        else {
            time += 1;
        }
    }
}

void print_results(const vector<Process*>& finished) {
    cout << "----------------------------------------------------------------------\n";
    cout << left << setw(12) << "Process_id" << setw(12) << "Queue_id" << setw(12) 
        << "priority" << setw(18) << "computing_time" << setw(18) << "turn_around time"<<'\n';
    cout << "----------------------------------------------------------------------\n";

    for (auto p : finished) {
        cout << left << setw(12) << p->process_id
            << setw(12) << p->final_queue
            << setw(12) << p->final_priority
            << setw(18) << p->computing_time
            << setw(18) << p->turnaround_time << "\n";
    }

    cout << "----------------------------------------------------------------------\n";

    double total_ntat = 0.0;
    for (auto p : finished) {
        total_ntat += static_cast<double>(p->turnaround_time) / p->computing_time;
    }

    double avg_ntat = total_ntat / finished.size();
    cout << fixed << setprecision(2);
    cout << "Average Normalized Turnaround Time: " << avg_ntat << "\n";
}

void cleanup(vector<Process*>& finished) {
    for (auto p : finished) {
        delete p;
    }
}
