/* 
 * File:   main.cpp
 * Author: albertoplaza
 *
 * Created on 5 de octubre de 2021, 10:00
 */

#include <iostream>
#include <cstdlib>
#include <chrono> 
#include "scd.h"
#include <mutex>
#include <thread>
using namespace std;
using namespace scd;

int num_lectores = 5,
        num_escritores = 2;

class Lec_Esc : public HoareMonitor {
private:
    int n_lec;
    bool escrib;
    CondVar lectura;
    CondVar escritura;

public:

    Lec_Esc() {
        n_lec = 0;
        escrib = false;
        lectura = newCondVar();
        escritura = newCondVar();
    }

    void ini_lectura() {
        if (escrib) 
            lectura.wait();
        n_lec++;
        lectura.signal();
        
    }

    void fin_lectura() {
        n_lec--;
        if (n_lec == 0)
            escritura.signal();
    }

    void ini_escritura() {
        if (n_lec > 0 || escrib) 
            escritura.wait();
        escrib = true;
        
    }

    void fin_escritura() {
        escrib = false;

        if (!lectura.empty())
            lectura.signal();
        else
            escritura.signal();

    }

};

void Lector(int n, MRef<Lec_Esc> monitor) {

    while (true) {
        
        this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));
        
        monitor->ini_lectura();
        cout <<"hebra " << n << " Leo" << endl;
        monitor->fin_lectura();
       
        
    }
}

void Escritor(int n, MRef<Lec_Esc> monitor) {

    while (true) {
        
        this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));
        monitor->ini_escritura();
        cout <<"hebra " << n << " Escribo" << endl;
        monitor->fin_escritura();
        
    }
    
}

int main(int argc, char** argv) {

    MRef<Lec_Esc> monitor = Create<Lec_Esc>();


    thread hebrasLectoras[num_lectores];
    thread hebrasEscritoras[num_escritores];

    for (int i = 0; i < num_lectores; i++)
        hebrasLectoras[i] = thread(Lector, i, monitor);

    for (int i = 0; i < num_escritores; i++)
        hebrasEscritoras[i] = thread(Escritor, i, monitor);

    for (int i = 0; i < num_lectores; i++)
        hebrasLectoras[i].join();

    for (int i = 0; i < num_escritores; i++)
        hebrasEscritoras[i].join();

    return 0;


}
