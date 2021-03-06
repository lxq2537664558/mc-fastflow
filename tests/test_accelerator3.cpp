/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ***************************************************************************
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  As a special exception, you may use this file as part of a free software
 *  library without restriction.  Specifically, if other files instantiate
 *  templates or use macros or inline functions from this file, or you compile
 *  this file and link it with other files to produce an executable, this
 *  file does not by itself cause the resulting executable to be covered by
 *  the GNU General Public License.  This exception does not however
 *  invalidate any other reasons why the executable file might be covered by
 *  the GNU General Public License.
 *
 ****************************************************************************
 */

/*
 * Very basic test for the FastFlow farm (without the collector) in the 
 * accelerator configuration.
 *
 */
#include <vector>
#include <iostream>
#include <ff/farm.hpp>
#include <ff/node.hpp>
  

using namespace ff;

// generic worker
class Worker: public ff_node {
public:
    void * svc(void * task) {
        int * t = (int *)task;
        std::cout << "[Worker] " << ff_node::get_my_id() 
                  << " received task " << *t << "\n";
        return task;
    }
    // I don't need the following for this test
    //int   svc_init() { return 0; }
    //void  svc_end() {}

};

// the gatherer filter
class Collector: public ff_node {
public:
    Collector(ff_farm<> * f):secondFarm(f) {}

    int svc_init() {
        if (secondFarm==NULL) return 0;
        else {
            std::cerr << "Starting 2 farm\n";
            secondFarm->run_then_freeze();
        }
        return 0;
    }

    void * svc(void * task) {   
        void * result=NULL;
        int * t = (int *)task;

        
        if (secondFarm) {
            std::cout << "[Farm Collector1] task received " << *t << "\n";
            secondFarm->offload(task);
            if (secondFarm->load_result_nb(&result)) {
                std::cerr << "result= " << *((int*)result) << "\n";
                delete ((int*)result);
                return GO_ON;
            }
        } else {
            std::cout << "[Farm Collector2] task received " << *t << "\n";
        }
        return task;
    }

    void svc_end() {
        if (secondFarm)  {
            std::cerr << "SENDING EOS and WAITING RESULTS\n";
            secondFarm->offload((void *)FF_EOS);

            void * result;
            while(secondFarm->load_result(&result)) {
                std::cerr << "result= " << *((int*)result) << "\n";
                delete ((int*)result);
            }
            secondFarm->wait();
        }
    }


private:
    ff_farm<> * secondFarm;

};

// the load-balancer filter 
/* 
class Emitter: public ff_node {
public:
    Emitter(int max_task):ntask(max_task) {
        std::cout << "Emitter set up\n";  
    };
    void * svc(void *intask) {     
        //std::cout << " EMITTER \n";
        return intask;   
    }
private:
    int ntask;
};
*/

int main(int argc, 
         char * argv[]) {

    if (argc<3) {
        std::cerr << "use: " 
                  << argv[0] 
                  << " nworkers streamlen\n";
        return -1;
    }
    
    int nworkers=atoi(argv[1]);
    int streamlen=atoi(argv[2]);

    if (nworkers<=0 || streamlen<=0) {
        std::cerr << "Wrong parameters values\n";
        return -1;
    }
    
    ff_farm<> farm(true /* accelerator set */);
    
    // Using standard emitter
    /*
      Emitter E(streamlen);
      farm.add_emitter(&E);
    */

    std::vector<ff_node *> w;
    for(int i=0;i<nworkers;++i) w.push_back(new Worker);
    farm.add_workers(w);


    

    ff_farm<> farm2(true);
    std::vector<ff_node *> w2;
    for(int i=0;i<nworkers;++i) w2.push_back(new Worker);
    farm2.add_workers(w2);
    Collector C2(NULL);
    farm2.add_collector(&C2);
    

    Collector C(&farm2);
    farm.add_collector(&C);   


    // Now run the accelator asynchronusly
    farm.run();
    std::cout << "[Main] Farm accelerator started\n";
    
    for (int i=0;i<streamlen;i++) {
        int * ii = new int(i);
        std::cout << "[Main] Offloading " << i << "\n"; 
        // Here offloading computation onto the farm
        farm.offload(ii); 
    }
    std::cout << "[Main] EOS arrived\n";
    void * eos = (void *)FF_EOS;
    farm.offload(eos);
    
    // Here join
    farm.wait();  

    std::cout << "[Main] Farm accelerator stopped\n";

    std::cerr << "[Main] DONE, time= " << farm.ffTime() << " (ms)\n";
    return 0;
}
