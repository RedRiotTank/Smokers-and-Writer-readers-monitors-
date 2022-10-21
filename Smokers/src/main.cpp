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

// ---------------- VAR GLOBALES --------------------


const int numFumadores = 3;


class Estanco : public HoareMonitor {
private:
    int ingrediente;
    bool mostradorVacio;
    bool fumador[numFumadores];
    
    CondVar puedePonerEnMostrador;
    CondVar fumadorpuedeCogerIngrediente[numFumadores];
public:
    Estanco(){
        ingrediente = -1;
        mostradorVacio = true;
        puedePonerEnMostrador = newCondVar();
        
        for(int i=0; i<numFumadores; i++){
            fumador[i] = false;
            fumadorpuedeCogerIngrediente[i] = newCondVar();
        }
        
    }
    
    
    void obtenerIngrediente(int i){ //i es el num de fumador o el num de ingrediente que esperan
        
        if(!fumador[i]){
            fumadorpuedeCogerIngrediente[i].wait();
        }
        
        cout << "El fumador " << i <<" coge el ingrediente" << endl;
        mostradorVacio = true;
        fumador[i]=false;
        puedePonerEnMostrador.signal();
        
        
        
    }
    
    void ponerIngrediente(int n){
        ingrediente = n;
        mostradorVacio = false;
        cout << "El estanquero ha puesto el ingrediente: " << ingrediente << endl;
        fumador[n]=true;
        fumadorpuedeCogerIngrediente[n].signal();
    }
    void esperarRecogidaIngrediente(){
        if(!mostradorVacio)
            puedePonerEnMostrador.wait();
    
    }
};


int ProducirIngrediente(){
    this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 200>()));
    int produccion = aleatorio<0, numFumadores - 1>();
    return produccion;
}

void fumar(int i){
    cout << "Fumador número " << i << ": comienza a fumar." << endl;
    this_thread::sleep_for( chrono::milliseconds( aleatorio<20,200>() ));
    cout << "Fumador número " << i << ": termina de fumar." << endl;
}

void Estanquero(MRef<Estanco> monitor){
    int ingre;
    
    while (true) {
        ingre = ProducirIngrediente();  //produce un ingrediente aleatorio hace una 
                                        //espera de duración aleatoria y devuelve * 
                                        //un número de ingrediente aleatorio)
        monitor->ponerIngrediente(ingre);//(se pone el ingrediente i en el mostrador)
        monitor->esperarRecogidaIngrediente();//(espera bloqueado ahsta que el mostrador está libre)
        
    }

}

void Fumador(int i, MRef<Estanco> monitor){
    while(true){
        monitor->obtenerIngrediente(i); //El fumador espera bloqueado a que su ingrediente
                                        //este disponible y luego lo retira del mostrador.
        fumar(i);                       //, (llamada a la funcion fumar 
                                        //que es una espera aleatoria)
    }
}







int main(int argc, char** argv) {

    MRef<Estanco> monitor = Create<Estanco>();


    thread hebrasFumadores[numFumadores];
    thread hebraEstanquero (Estanquero,monitor);

    for (int i = 0; i < numFumadores; i++)
        hebrasFumadores[i] = thread(Fumador, i, monitor);

   

    for (int i = 0; i < numFumadores; i++)
        hebrasFumadores[i].join();

    hebraEstanquero.join();

    return 0;


}