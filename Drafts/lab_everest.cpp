/*$TET$$header*/
/*--------------------------------------------------------------------------*/
/*  Copyright 2020 Sergei Vostokin                                          */
/*                                                                          */
/*  Licensed under the Apache License, Version 2.0 (the "License");         */
/*  you may not use this file except in compliance with the License.        */
/*  You may obtain a copy of the License at                                 */
/*                                                                          */
/*  http://www.apache.org/licenses/LICENSE-2.0                              */
/*                                                                          */
/*  Unless required by applicable law or agreed to in writing, software     */
/*  distributed under the License is distributed on an "AS IS" BASIS,       */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*/
/*  See the License for the specific language governing permissions and     */
/*  limitations under the License.                                          */
/*--------------------------------------------------------------------------*/

#define _CRT_SECURE_NO_DEPRECATE

#include <templet.hpp>
#include <everest.hpp>
#include <string>
#include <iostream>

// элемент последовательности
class sequence_element : public templet::message {
public:
	sequence_element(templet::actor*a=0, templet::message_adaptor ma=0) :templet::message(a, ma) {}
    int n; // проверяемое число
};
/*$TET$*/

#pragma templet !Begin(out!sequence_element)

struct Begin :public templet::actor {
	static void on_out_adapter(templet::actor*a, templet::message*m) {
		((Begin*)a)->on_out(*(sequence_element*)m);}

	Begin(templet::engine&e) :Begin() {
		Begin::engines(e);
	}

	Begin() :templet::actor(true),
		out(this, &on_out_adapter)
	{
/*$TET$Begin$Begin*/
        stages_num = 2; // мы намеренно пропустили 0 и 1: 0 - исключительная ситуация, 1 - проверять нет смысла.
/*$TET$*/
	}

	void engines(templet::engine&e) {
		templet::actor::engine(e);
/*$TET$Begin$engines*/
/*$TET$*/
	}

	void start() {
/*$TET$Begin$start*/
        on_out(out); // первым делом генерируем первое сообщение, которое инициирует рассылку элементов последовательности
/*$TET$*/
	}

	inline void on_out(sequence_element&m) {
/*$TET$Begin$out*/

        //cout << "--- Отправка элемента последовательности из начального актора" << endl;
        if (stages_num < max_elements_num)
        {
            out.n = stages_num;
            cout << "--- Отправка элемента последовательности "<< stages_num << " из начального актора Begin" << endl;
            stages_num++;
            // cout << out.n << endl; // remove
            out.send();
        }
        
/*$TET$*/
	}

	sequence_element out;

/*$TET$Begin$$footer*/

    int stages_num; // значение проверяемого элемента = итератору цикла
    int max_elements_num; // максимальное число элементов в последовательности

/*$TET$*/
};

#pragma templet CheckStage(in?sequence_element,out!sequence_element,t:everest)

struct CheckStage :public templet::actor {
	static void on_in_adapter(templet::actor*a, templet::message*m) {
		((CheckStage*)a)->on_in(*(sequence_element*)m);}
	static void on_out_adapter(templet::actor*a, templet::message*m) {
		((CheckStage*)a)->on_out(*(sequence_element*)m);}
	static void on_t_adapter(templet::actor*a, templet::task*t) {
		((CheckStage*)a)->on_t(*(templet::everest_task*)t);}

	CheckStage(templet::engine&e,templet::everest_engine&te_everest) :CheckStage() {
		CheckStage::engines(e,te_everest);
	}

	CheckStage() :templet::actor(false),
		out(this, &on_out_adapter),
		t(this, &on_t_adapter)
	{
/*$TET$CheckStage$CheckStage*/
        _in = 0;
        saved_number = 0;
        t.app_id("6616cdc91200001600e5e77a");
/*$TET$*/
	}

	void engines(templet::engine&e,templet::everest_engine&te_everest) {
		templet::actor::engine(e);
		t.engine(te_everest);
/*$TET$CheckStage$engines*/
/*$TET$*/
	}

	inline void on_in(sequence_element&m) {
/*$TET$CheckStage$in*/
        cout << "--- CheckStage " << stage_id << endl;
        _in = &m;
        check_element();
/*$TET$*/
	}

	inline void on_out(sequence_element&m) {
/*$TET$CheckStage$out*/
        check_element();
/*$TET$*/
	}

	inline void on_t(templet::everest_task&t) {
/*$TET$CheckStage$t*/
        
        // результат выаолнения задачи Everest
        json response = t.result();
        string res = response["isDivided"].get<string>();
        char isDivided = res[0];
        
        //cout << "------ Пришел ответ от Everest: isDivied = " << isDivided << endl;
        
        if ('0'==isDivided)
        {
            cout << "------ " << _in->n << " не кратно числу " << saved_number << endl;
            out.n = _in->n;
            _in->send();
            out.send();
        }
        else
        {
            cout << "------ " << _in->n << " кратно числу " << saved_number << endl;
            _in->send();
        }
        
/*$TET$*/
	}

	void in(sequence_element&m) { m.bind(this, &on_in_adapter); }
	sequence_element out;
	templet::everest_task t;

/*$TET$CheckStage$$footer*/

    void check_element()
    {
        if (access(_in) && access(out)){
            cout << "------ Проверка элемента последовательности " << _in -> n << " в акторе со stage_id = " << stage_id << endl;
            if (saved_number == 0)
            {
                saved_number = _in->n;
                cout << "------ Сохранение элемента " << saved_number << " в акторе со stage_id = " << stage_id << endl;
                
                cout << "---------------------------------"<< endl;
                cout << "|   НАЙДЕНО ПРОСТОЕ ЧИСЛО: " << saved_number << "   |"<< endl;
                cout << "---------------------------------"<< endl;
                
                _in->send();
                
            }
            else
            {
                // параметры задачи, соответствующие описанию на Everest
                json request;
                request["name"] = "8v_lab_resourse";
                request["inputs"]["dividend"] = _in->n;
                request["inputs"]["divisor"] = saved_number;
        
        		if (t.submit(request)) std::cout << "--начало выполнения задачи Everest" << std::endl;
        		else std::cout << "--ошибка исполнения задачи Everest" << std::endl;
            }
        }    
    }
    int stage_id; // идентификатор этапа (для проверки)
    sequence_element* _in; // обрабатываемое входящее сообщение
    int saved_number; // сохраненное при получении актором первого сообщения и кратность которому мы проверяем при полученииследующих сообщений

/*$TET$*/
};

#pragma templet End(in?sequence_element)

struct End :public templet::actor {
	static void on_in_adapter(templet::actor*a, templet::message*m) {
		((End*)a)->on_in(*(sequence_element*)m);}

	End(templet::engine&e) :End() {
		End::engines(e);
	}

	End() :templet::actor(false)
	{
/*$TET$End$End*/
        last_number = 0;
/*$TET$*/
	}

	void engines(templet::engine&e) {
		templet::actor::engine(e);
/*$TET$End$engines*/
/*$TET$*/
	}

	inline void on_in(sequence_element&m) {
/*$TET$End$in*/
        last_number = m.n;
        cout << "--- Сохранение последнего элемента в акторе End " << last_number << endl;
        
        cout << "---------------------------------"<< endl;
        cout << "|   ПОСЛЕДНЕЕ ПРОСТОЕ ЧИСЛО: "    << last_number << "   |"<< endl;
        cout << "---------------------------------"<< endl;
        stop();
/*$TET$*/
	}

	void in(sequence_element&m) { m.bind(this, &on_in_adapter); }

/*$TET$End$$footer*/
    int last_number; // 
/*$TET$*/
};


/*$TET$$footer*/
int main()
{
	templet::engine eng;
	templet::everest_engine teng("token");

	if (!teng) {
		std::cout << "task engine is not connected to the Everest server..." << std::endl;
		return EXIT_FAILURE;
	}
    
    int N = 4;

	Begin beginStage(eng);
    CheckStage stages[N];
    End   endStage(eng);

    beginStage.max_elements_num = 15;

    sequence_element* last_check_stage = &beginStage.out; // в начальный актор передаем ссылку на собщение первого проверяющего актора в цепочке
    endStage.in(stages[N-1].out); // в финальный актор передаем ссылку на собщение последнего проверяющего в цепочке

    // каждому актору в цепочке присваиваем ссылку на сообщение следующего и вспомогательный идентификатор (для отладки)
    for(int i=0; i<N; i++){
        stages[i].engines(eng, teng);  

        stages[i].stage_id = i; 
        stages[i].in(*last_check_stage); 
        last_check_stage = &stages[i].out;
    }

    eng.start();
    
try_continue:
	teng.run();

	if (eng.stopped()) {
		std::cout << "Успешное выполнение." << std::endl;
		

		return EXIT_SUCCESS;
	}

    
    // обработка ошибок - оставила из исходника
    
	static int recovery_tries = 3;
	templet::everest_error cntxt;

	if (teng.error(&cntxt)) {
		std::cout << "...task engine error..." << std::endl;

		if (recovery_tries--) {
			std::cout << "error information:" << std::endl;

			std::cout << "type ID : " << *cntxt._type << std::endl;
			std::cout << "HTML response code : " << cntxt._code << std::endl;
			std::cout << "HTML response : " << cntxt._response << std::endl;
			std::cout << "task input : " << cntxt._task_input << std::endl;

			// trying to fix an error
			switch (*cntxt._type) {
				case templet::everest_error::NOT_CONNECTED:
				{
					std::cout << "error string : NOT_CONNECTED" << std::endl;
					std::cout << "the task engine is not initialized properly -- fatal error, exiting" << std::endl;
					return EXIT_FAILURE;
				}
				case templet::everest_error::SUBMIT_FAILED:
				{
					std::cout << "error string : SUBMIT_FAILED" << std::endl;
					std::cout << "resubmitting the task" << std::endl;
					json input = json::parse(cntxt._task_input);
					// here you may fix something in the input 
					cntxt._task->resubmit(input);
					break;
				}
				case templet::everest_error::TASK_CHECK_FAILED:
				{
					std::cout << "error string : TASK_CHECK_FAILED" << std::endl;
					std::cout << "trying to check the task status again" << std::endl;
					*cntxt._type = templet::everest_error::NOT_ERROR;
					break;
				}
				case templet::everest_error::TASK_FAILED_OR_CANCELLED:
				{
					std::cout << "error string : TASK_FAILED_OR_CANCELLED" << std::endl;
					std::cout << "resubmitting the task" << std::endl;
					json input = json::parse(cntxt._task_input);
					// here you may fix something in the input 
					cntxt._task->resubmit(input);
				}
			}
	
			goto try_continue;
		}
	}
	else 
		std::cout << "logical error" << std::endl;
	
	return EXIT_FAILURE;
}
/*$TET$*/
