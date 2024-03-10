#include "lab.hpp"
const int N = 4; // число звеньев в цепочке проверяющих акторов

int main (){
        
    engine eng;
    basesim_engine teng;
        
    Begin beginStage(eng);
    CheckStage stages[N];
    End   endStage(eng);
        
    beginStage.max_elements_num = 30;
        
    sequence_element* last_check_stage = &beginStage.out; // в начальный актор передаем ссылку на первый проверяющий в цепочке
    endStage.in(stages[N-1].out); // в финальный актор передаем ссылку на последний проверяющий в цепочке
    
    // каждому актору в цепочке присваиваем ссылку на следующий и вспомогательный идентификатор (для отладки)
    for(int i=0; i<N; i++){
        stages[i].engines(eng, teng);  
        
        stages[i].stage_id = i; 
        stages[i].in(*last_check_stage); 
        last_check_stage = &stages[i].out;
    }
    
    eng.start();
    teng.run();
        
    cout << (eng.stopped() ? "Успешное выполнение" : "Возникла ошибка");
}
