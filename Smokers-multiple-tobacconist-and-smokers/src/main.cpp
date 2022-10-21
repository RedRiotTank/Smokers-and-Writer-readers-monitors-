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
const int numEstanqueros = 2;
const int maxIngredientesEnMostrador = 4;

const int 
        minProd = 20,
        maxProd = 200,
        minFum = 200,
        maxFum = 200;


class Estanco : public HoareMonitor {
private:
    int ingrediente;
    
    bool mostradorVacio;
    int ingredientesEnMostrador = 0;
    
    bool fumadorTieneIngredienteDisponible[numFumadores];
    
    CondVar puedePonerEnMostrador;
    CondVar fumadorpuedeCogerIngrediente[numFumadores];
public:
    Estanco(){
        ingrediente = -1;
        mostradorVacio = true;
        puedePonerEnMostrador = newCondVar();
        
        for(int i=0; i<numFumadores; i++){
            fumadorTieneIngredienteDisponible[i] = false;
            fumadorpuedeCogerIngrediente[i] = newCondVar();
        }
        
    }
    
    
    void obtenerIngrediente(int i){ //i es el num de fumador o el num de ingrediente que esperan
        
        if(!fumadorTieneIngredienteDisponible[i]){
            fumadorpuedeCogerIngrediente[i].wait();
        }
        
        ingredientesEnMostrador--;
        
        cout << "El fumador " << i <<" coge el ingrediente" << endl;
        
        
        fumadorTieneIngredienteDisponible[i]=false;
        
        if(ingredientesEnMostrador == 0)
            mostradorVacio = true;
        
        puedePonerEnMostrador.signal();
        
        
        
    }
    
    void ponerIngrediente(int num_estanquero,int ingrediente){
        
        if(ingredientesEnMostrador == maxIngredientesEnMostrador)
            puedePonerEnMostrador.wait();
        
        ingredientesEnMostrador++;
        mostradorVacio = false;

        cout << "El estanquero"<< num_estanquero<<" ha puesto el ingrediente: " << ingrediente << endl;
        fumadorTieneIngredienteDisponible[ingrediente]=true;
        fumadorpuedeCogerIngrediente[ingrediente].signal();
    }
    void esperarRecogidaIngrediente(){
        if(!mostradorVacio)
            puedePonerEnMostrador.wait();
    
    }
};


int ProducirIngrediente(){
    this_thread::sleep_for(chrono::milliseconds(aleatorio<minProd, maxProd>()));
    int produccion = aleatorio<0, numFumadores - 1>();
    return produccion;
}

void fumar(int i){
    cout << "Fumador número " << i << ": comienza a fumar." << endl;
    this_thread::sleep_for( chrono::milliseconds( aleatorio<minFum,maxFum>() ));
    cout << "Fumador número " << i << ": termina de fumar." << endl;
}

void Estanquero(int num_estanquero, MRef<Estanco> monitor){
    int ingre;
    
    while (true) {
        ingre = ProducirIngrediente();  //produce un ingrediente aleatorio hace una 
                                        //espera de duración aleatoria y devuelve * 
                                        //un número de ingrediente aleatorio)
        monitor->ponerIngrediente(num_estanquero,ingre);//(se pone el ingrediente i en el mostrador)
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
    thread hebrasEstanquero[numEstanqueros];
    
    for(int i=0; i < numEstanqueros; i++)
        hebrasEstanquero[i] = thread(Estanquero,i,monitor);

    for (int i = 0; i < numFumadores; i++)
        hebrasFumadores[i] = thread(Fumador, i, monitor);

   

    for (int i = 0; i < numFumadores; i++)
        hebrasFumadores[i].join();

   for (int i = 0; i < numFumadores; i++)
        hebrasFumadores[i].join();

    return 0;


}