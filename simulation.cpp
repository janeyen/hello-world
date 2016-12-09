#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <random>
#include <vector>
using namespace std;

const int k = 8; // set num of pods
// declare classes
class core;
class aggregation;
class pkt;

// define classes
class core {
public:
  core() {
    port_num = k;
    row = 0;
    col = 0;
    for (int m = 0; m < k; m++) {
      port_down[m] = NULL;
      port_active[m] = true;
    }
    upgrade = false;
    on = true;
  }
  void add_to_buffer(pkt &packet) { buffer.push_back(packet); }
  void buffer_shuffle() { random_shuffle(buffer.begin(), buffer.end()); }
  void erase_first() { buffer.erase(buffer.begin()); }
  void clear_buffer() { buffer.clear(); }
  // set parameters function
  void set_par(int i, int j) {
    row = i;
    col = j;
  }
  void set_port_down(aggregation *agg, int port_num) {
    port_down[port_num] = agg;
  }
  void set_on(int p) { port_active[p] = true; }
  void set_off(int p) { port_active[p] = false; }
  void set_core_on() { on = true; }
  void set_core_off() { on = false; }
  void set_upgrade() { upgrade = true; }
  // get value functions
  int get_row() { return row; }
  int get_col() { return col; }
  pkt &get_first() { return buffer[0]; }
  int get_buffer_size() { return buffer.size(); }
  aggregation *get_agg(int port) { return port_down[port]; }
  bool get_upgrade() { return upgrade; }
  bool get_on() { return on; }

private:
  int port_num;
  int row;
  int col;
  aggregation *port_down[k];
  vector<pkt> buffer;
  bool port_active[k];
  bool upgrade;
  bool on;
};

class aggregation {
public:
  aggregation() {
    belong_pod = 0;
    index = 0;
    port_num = k;
    for (int m = 0; m < k / 2; m++) {
      port_up[m] = NULL;
      port_active[m] = true;
    }
    upgrade = false;
    on = true;
  }
  void buffer_shuffle() {
    random_shuffle(buffer_sent.begin(), buffer_sent.end());
  }
  void clear_buffer() { buffer_sent.clear(); }
  vector<int> check_avail() {
    vector<int> record;
    for (int p = 0; p < k / 2; p++) {
      if (port_active[p] == true)
        record.push_back(p);
    }
    return record;
  }
  void add_to_buffer(pkt &packet) { buffer_received.push_back(packet); }
  // set parameter functions
  void set_par(int i, int j) {
    belong_pod = i;
    index = j;
  }
  void set_on(int p) { port_active[p] = true; }
  void set_off(int p) { port_active[p] = false; }
  void set_agg_on() { on = true; }
  void set_agg_off() { on = false; }
  void set_upgrade() { upgrade = true; }
  void alloc(pkt &packet) { buffer_sent.push_back(packet); }
  void set_port_up(core *link_core, int port) { port_up[port] = link_core; }
  // get value functions
  int get_belong_pod() { return belong_pod; }
  int get_buffer_sent_size() { return buffer_sent.size(); }
  int get_buffer_received_size() { return buffer_received.size(); }
  core *get_core(int port) { return port_up[port]; }
  pkt &get_first() { return buffer_sent[0]; }
  void erase_first() { // TODO move it to upper b/c it's not get-functions
    buffer_sent.erase(buffer_sent.begin());
  }
  int get_index() { return index; }
  bool get_upgrade() { return upgrade; }
  bool get_on() { return on; }
  // calculate available number of port
  /*	int get_avail_port_num(){
                  int count=0;
                  for(int m=0;m<k/2;m++){
                          if(port_active[m]==true) count++;
                  }
                  return count;
          }*/

private:
  int belong_pod;
  int index;
  int port_num;
  core *port_up[k / 2];
  bool port_active[k / 2];
  bool upgrade;
  bool on;
  vector<pkt> buffer_sent;
  vector<pkt> buffer_received;
};

class pkt {
public:
  pkt() {
    dest_pod = 0;
    dest_agg = 0;
    src_pod = 0;
    src_agg = 0;
    success = false;
  }
  // set value
  void set_pkt_pod(int source, int destination) {
    dest_pod = destination;
    src_pod = source;
  }
  void set_pkt_src_agg(int source_agg) { src_agg = source_agg; }
  void set_pkt_dest_agg(int destination_agg) { dest_agg = destination_agg; }
  void set_success() { success = true; }
  // get value
  int get_dest_pod() { return dest_pod; }
  int get_src_pod() { return src_pod; }
  int get_src_agg() { return src_agg; }

private:
  int dest_pod;
  int dest_agg;
  int src_pod;
  int src_agg;
  bool success;
};

// Fat-tree topology link connection
void FatTree(vector<vector<core>> &type1, vector<vector<aggregation>> &type2) {
  for (int m = 0; m < k / 2; m++) {
    for (int n = 0; n < k / 2; n++) {
      for (int p = 0; p < k; p++) {
        type1[m][n].set_port_down(&(type2[p][m]), p);
        type2[p][m].set_port_up(&(type1[m][n]), n);
        //				cout<<"[row,col]:\t"<<type1[m][n].get_row()<<","<<type1[m][n].get_col()<<"\tport"<<p<<":
        //"<<type1[m][n].get_port(p)<<endl;
      }
    }
  }
}

// F10 topology link connection
void F10(vector<vector<core>> &type1, vector<vector<aggregation>> &type2) {
  for (int m = 0; m < k / 2; m++) {
    for (int n = 0; n < k / 2; n++) {
      for (int p = 0; p < k; p++) {
        if (p % 2 == 0)
          type1[m][n].set_port_down(&(type2[p][m]),
                                    p); // A-type tree is row-like
        else
          type1[m][n].set_port_down(&(type2[p][n]),
                                    p); // B-type tree is col-like
      }
    }
  }
  for (int m = 0; m < k; m++) {
    if (m % 2 == 0) {
      for (int n = 0; n < k / 2; n++) {
        for (int port = 0; port < k / 2; port++) {
          type2[m][n].set_port_up(&type1[n][port], port);
        }
      }
    } else {
      for (int n = 0; n < k / 2; n++) {
        for (int port = 0; port < k / 2; port++) {
          type2[m][n].set_port_up(&type1[port][n], port);
        }
      }
    }
  }
}

bool FatTree_schedule(vector<vector<core>> &layerOne,
                      vector<vector<aggregation>> &layerTwo, float util) {
  int token = 0;
  bool change = false;
  for (int m = 0; m < k / 2; m++) {
    if (((layerTwo[0][m]).get_upgrade() == false)) {
      token = m;
      change = true;
      break;
    }
  }
  int num_row = floor((1 - util) * k / 2);
  if (change == true) {
    cout << "token:" << token << "; num_row" << num_row << endl;
    for (int m = token; m < token + num_row; m++) {
      if (m < k / 2) {
        for (int n = 0; n < k / 2; n++) {
          layerOne[m][n].set_upgrade();
          layerOne[m][n].set_core_off();
          for (int port = 0; port < k; port++) { // set all links off
            layerOne[m][n].set_off(port);
          }
        }
      }
    }
    for (int m = 0; m < k; m++) {
      for (int n = token; n < token + num_row;
           n++) { // Just to examine if it would have trouble TODO
        if (n < k / 2) {
          layerTwo[m][n].set_upgrade();
          layerTwo[m][n].set_agg_off();
          for (int port = 0; port < k / 2; port++) {
            layerTwo[m][n].set_off(port);
          }
        }
      }
    }
  }
  return change;
}

bool F10_schedule(vector<vector<core>> &layerOne,
                  vector<vector<aggregation>> &layerTwo, float util) {
  int token = 0;
  bool change = false;
  int num_row = floor((1 - sqrt(util)) * k / 2);
  for (int m = 0; m < k / 2; m++) {
    if ((layerOne[m][m].get_upgrade() == false) && (num_row > 0)) {
      token = m;
      change = true;
      break;
    }
  }

  if (change == true) {
    cout << "token:" << token << "; num_row" << num_row << endl;
    for (int m = token; m < token + num_row; m++) {
      if (m < k / 2) {
        for (int n = 0; n < k / 2; n++) {
          // deal with row part
          //        if (layerOne[m][n].get_upgrade() == false) {
          layerOne[m][n].set_upgrade();
          layerOne[m][n].set_core_off();
          for (int port = 0; port < k; port++) {
            layerOne[m][n].set_off(port);
          }
          //		cout<<"Core "<<m<<","<<n<<"upgrade"<<endl;
          //        }
          // deal with row part
          //        if (layerOne[n][m].get_upgrade() == false) {
          layerOne[n][m].set_upgrade();
          layerOne[n][m].set_core_off();
          for (int port = 0; port < k; port++) {
            layerOne[n][m].set_off(port);
          }
          //       					cout<<"Core
          //       "<<n<<","<<m<<"upgrade"<<endl;
          //        }
        }
      }
    }
    for (int m = 0; m < k; m++) {
      for (int n = 0; n < k / 2; n++) {
        if ((n >= token) && (n < token + num_row)) {
          layerTwo[m][n].set_upgrade();
          layerTwo[m][n].set_agg_off();
          for (int port = 0; port < k / 2; port++) {
            layerTwo[m][n].set_off(port);
          }
        } else if (num_row != 0) {
          for (int port = token; (port < token + num_row) && (port < k / 2);
               port++) {
            layerTwo[m][n].set_off(port);
          }
        }
      }
    }
  }
  return change;
}

void FatTree_resume(vector<vector<core>> &layerOne,
                    vector<vector<aggregation>> &layerTwo, float util) {
  int token = 0;
  bool get = false;
  int num_row = floor((1 - util) * k / 2);
  for (int m = 0; m < k / 2; m++) {
    if ((get == false) && ((layerTwo[0][m]).get_on() == false)) {
      token = m;
      get = true;
      break;
    }
  }
  for (int m = token; m < token + num_row; m++) {
    if (m < k / 2) {
      for (int n = 0; n < k / 2; n++) {
        layerOne[m][n].set_core_on();
        //	cout<<"resume "<<m<<", "<<n<<" core"<<endl;
        for (int port = 0; port < k; port++) {
          layerOne[m][n].set_on(port);
        }
      }
    }
  }
  for (int m = 0; m < k; m++) {
    for (int n = token; n < token + num_row; n++) {
      if (n < k / 2) {
        layerTwo[m][n].set_agg_on();
        for (int port = 0; port < k / 2; port++) {
          layerTwo[m][n].set_on(port);
        }
      }
    }
  }
}
void F10_resume(vector<vector<core>> &layerOne,
                vector<vector<aggregation>> &layerTwo, float util) {
  int token = 0;
  bool get = false;
  int num_row = floor((1 - sqrt(util)) * k / 2);
  for (int m = 0; m < k / 2; m++) {
    if ((get == false) && (layerOne[m][m].get_on() == false)) {
      token = m;
      get = true;
      break;
    }
  }
  // if(change==true){
  for (int m = token; m < token + num_row; m++) {
    if (m < k / 2) {
      for (int n = 0; n < k / 2; n++) {
        // deal with row part
        //      if (layerOne[m][n].get_on() == false) {
        layerOne[m][n].set_core_on();
        for (int port = 0; port < k; port++) {
          layerOne[m][n].set_on(port);
        }
        //     					cout<<"Core
        //     "<<m<<","<<n<<"resume"<<endl;
        //      }
        // deal with row part
        //      if (layerOne[n][m].get_on() == false) {
        layerOne[n][m].set_core_on();
        for (int port = 0; port < k; port++) {
          layerOne[n][m].set_on(port);
        }
        //   					cout<<"Core
        //   "<<n<<","<<m<<"resume"<<endl;
        //      }
      }
    }
  }
  for (int m = 0; m < k; m++) {
    for (int n = 0; n < k / 2; n++) {
      if ((n >= token) && (n < token + num_row)) {
        layerTwo[m][n].set_agg_on();
        for (int port = 0; port < k / 2; port++) {
          layerTwo[m][n].set_on(port);
        }
      } else if (num_row != 0) {
        for (int port = token; (port < token + num_row) && (port < k / 2);
             port++) {
          layerTwo[m][n].set_on(port);
        }
      }
    }
  }
}

vector<vector<pkt>> generate_pkt(int num_pkt_pod,
                                 vector<vector<aggregation>> &layerTwo) {
  // generate packets in a second
  srand(time(NULL));
  vector<vector<pkt>> packets(k, vector<pkt>(num_pkt_pod));
  for (int q = 0; q < k; q++) {
    //    int n = rand() % k;		//	TODO 2 alternative here
    int n = q + 1;
    int m = 0;
    for (; m < num_pkt_pod;) {
      for (; n < k; n++) {
        if (m < num_pkt_pod) {
          if (n != q) {
            packets[q][m].set_pkt_pod(q, n);
            m++;
          } else
            continue;
        } else
          break;
      }
      n = 0;
    }
  }
  // set_src_agg			TODO revise so that it can detect the
  // inactive
  // agg
  for (int q = 0; q < k; q++) {
    vector<int> agg_active_record;
    for (int n = 0; (n < k / 2); n++) {
      if (layerTwo[q][n].get_on() == true)
        agg_active_record.push_back(n);
    }
    int avail = agg_active_record.size();
    if (avail > 0) {
      for (int m = 0; m < num_pkt_pod;) {
        int tmp = 0; // changed
        for (; tmp < avail; tmp++) {
          if (m < num_pkt_pod) {
            packets[q][m].set_pkt_src_agg(agg_active_record[tmp]);
            m++;
          } else
            break;
        }
        tmp = 0;
      }
    }
  }
  return packets;
}

void allocate_pkt(vector<vector<pkt>> &packets,
                  vector<vector<aggregation>> &layerTwo) {
  for (int m = 0; m < k; m++) {
    for (int n = 0; n < packets[m].size(); n++) {
      int src_pod = packets[m][n].get_src_pod();
      int src_agg = packets[m][n].get_src_agg();
      layerTwo[src_pod][src_agg].alloc(packets[m][n]);
    }
  }
}

void send_traffic(vector<vector<core>> &layerOne,
                  vector<vector<aggregation>> &layerTwo, int link_capacity,
                  int pkt_size) {
  //[TODO]
  int count = 0;
  int transmit_max = link_capacity / pkt_size;
  for (int m = 0; m < k; m++) { // layerTwo allocate first
    for (int n = 0; n < k / 2; n++) {
      if (layerTwo[m][n].get_on() == true) {
        //        layerTwo[m][n].buffer_shuffle(); // move shuffle to other
        //        place
        vector<int> port_info = layerTwo[m][n].check_avail();
        int avail = port_info.size();
        if ((avail == 0) && (layerTwo[m][n].get_on() == true))
          cout << "there's something wrong." << endl;
        if (avail > 0) {
          int pkt_to_sent = layerTwo[m][n].get_buffer_sent_size();
          int sent_max = int(pkt_to_sent / avail) + 1;
          int sent = min(transmit_max, sent_max);
          //[alternative] implement modulo-N hash here
          // round-robin
          vector<int> counter;
          for (int tmp = 0; tmp < avail; tmp++) {
            counter.push_back(0);
          }
          int token = 0;
          while (pkt_to_sent > 0) {
            //            int token = rand() % avail;
            if (counter[token] < transmit_max) {
              int port = port_info[token];
              core *tmp = layerTwo[m][n].get_core(port);
              (*tmp).add_to_buffer(layerTwo[m][n].get_first());
              layerTwo[m][n].erase_first();
              pkt_to_sent--;
              counter[token]++;
              token++;
              if (token == avail)
                token = 0;
            } else
              break;

            /*  if (counter[token] < transmit_max) {//change
                int port = port_info[token];
                core *tmp = layerTwo[m][n].get_core(port); // get the linked
              core
                (*tmp).add_to_buffer(layerTwo[m][n].get_first());
                layerTwo[m][n].erase_first();
                counter[token]++;
                pkt_to_sent--; // deal with the situation that the link capacity
                               // cannot handle the number of pkt_to_sent
              } else {
                avail--;
                port_info.erase(port_info.begin() + token);
                counter.erase(counter.begin() + token);
              }*/
          }
        }
      }
    }
  }
  for (int m = 0; m < k / 2; m++) {
    for (int n = 0; n < k / 2; n++) {
      if (layerOne[m][n].get_on() == true) {
        layerOne[m][n].buffer_shuffle();
        int pkt_to_sent = layerOne[m][n].get_buffer_size();
        int port =
            (layerOne[m][n].get_first())
                .get_dest_pod(); // destination pod index equals to the port num
        vector<int> counter;
        for (int tmp = 0; tmp < k; tmp++) {
          counter.push_back(0);
        }
        while (pkt_to_sent > 0) {
          int port = (layerOne[m][n].get_first()).get_dest_pod();
          if (counter[port] < transmit_max) {
            aggregation *agg = layerOne[m][n].get_agg(port); // get linked agg
            (*agg).add_to_buffer(layerOne[m][n].get_first());
            (layerOne[m][n].get_first()).set_pkt_dest_agg((*agg).get_index());
            (layerOne[m][n].get_first()).set_success();
            counter[port]++;
            count++;
          } else { /* cout<<"layerOne"<<" m:"<<m<<" n: "<<n<<endl;cout<<"certain
                      port "<<port<<" overwhelmed"<<endl;*/
          }
          pkt_to_sent--;
          layerOne[m][n].erase_first();
        }
      }
    }
  }
  //	cout<<"count: "<<count<<endl;
}
int count_success_pkt(vector<vector<aggregation>> &layerTwo) {
  int counter = 0;
  for (int m = 0; m < k; m++) {
    for (int n = 0; n < k / 2; n++) {
      counter += layerTwo[m][n].get_buffer_received_size();
    }
  }
  return counter;
}

int main() {
  int link_capacity = pow(10, 4); // 10GB link capacity
  int pkt_size = 10;              // 10MB pkt_size
  int simulation_sec = 10;
  float util = 0.5;
  int step = 0;
  float pod_utilization[k] = {util};
  int num_pkt_pod = k * k / 4 * (floor(link_capacity / pkt_size * util));

  vector<vector<core>> layerOne(k / 2, vector<core>(k / 2));
  vector<vector<aggregation>> layerTwo(k, vector<aggregation>(k / 2));

  for (int m = 0; m < k / 2; m++) {
    for (int n = 0; n < k / 2; n++) {
      layerOne[m][n].set_par(m, n);
    }
  }

  for (int m = 0; m < k; m++) {
    for (int n = 0; n < k / 2; n++) {
      layerTwo[m][n].set_par(m, n);
    }
  }
  FatTree(layerOne, layerTwo); // Fat-tree link connection
  //  F10(layerOne, layerTwo); // F10 link connection
  // back up info
  queue<vector<vector<pkt>>> pkt_record;
  for (int time = 0; time < simulation_sec + 2; time++) {
    bool change = FatTree_schedule(layerOne, layerTwo, util);
    //    bool change = F10_schedule(layerOne, layerTwo, util);
    if (time < simulation_sec) {
      vector<vector<pkt>> packets = generate_pkt(num_pkt_pod, layerTwo);
      cout << time << " sec generates total # of pkts: " << k * num_pkt_pod
           << endl;
      pkt_record.push(packets);
      allocate_pkt(packets, layerTwo);
    }
    send_traffic(layerOne, layerTwo, link_capacity, pkt_size);
    cout << "total received # of packets: " << count_success_pkt(layerTwo)
         << endl;
    if (change == true)
      step++;
    if (change == true)
      FatTree_resume(layerOne, layerTwo, util);
    //    if (change == true) F10_resume(layerOne, layerTwo, util);
  }

  /*	for(int time=simulation_sec; time>0; time--){
                  vector<vector<pkt>> packet_pattern = pkt_record.back();
                  for(int m=0; m<k;m++){
                          for(int n=0; n<num_pkt_pod;n++){
                                  cout<<packet_pattern[m][n].get_dest_pod();
                          }
                          cout<<endl;
                  }
                  pkt_record.pop();
                  cout<<"time: "<<time<<" sec"<<endl;
          }*/

  cout << "steps: " << step << endl;
  return 0;
}
