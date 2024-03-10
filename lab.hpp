/*$TET$$header*/
/*--------------------------------------------------------------------------*/
/*  Copyright 2023 Sergei Vostokin                                          */
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

#pragma once

#include <templet.hpp>
#include <iostream>
#include <basesim.hpp>

using namespace templet;
using namespace std;

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

        cout << "--- Отправка элемента последовательности из начального актора" << endl;
        if (stages_num < max_stages_num)
        {
            out.n = stages_num;
            stages_num++;
            // cout << out.n << endl; // remove
            out.send();
        }
        
/*$TET$*/
	}

	sequence_element out;

/*$TET$Begin$$footer*/

    int stages_num;
    int max_stages_num;

/*$TET$*/
};

#pragma templet CheckStage(in?sequence_element,out!sequence_element,t:basesim)

struct CheckStage :public templet::actor {
	static void on_in_adapter(templet::actor*a, templet::message*m) {
		((CheckStage*)a)->on_in(*(sequence_element*)m);}
	static void on_out_adapter(templet::actor*a, templet::message*m) {
		((CheckStage*)a)->on_out(*(sequence_element*)m);}
	static void on_t_adapter(templet::actor*a, templet::task*t) {
		((CheckStage*)a)->on_t(*(templet::basesim_task*)t);}

	CheckStage(templet::engine&e,templet::basesim_engine&te_basesim) :CheckStage() {
		CheckStage::engines(e,te_basesim);
	}

	CheckStage() :templet::actor(false),
		out(this, &on_out_adapter),
		t(this, &on_t_adapter)
	{
/*$TET$CheckStage$CheckStage*/
        _in = 0;
        saved_number = 0;
/*$TET$*/
	}

	void engines(templet::engine&e,templet::basesim_engine&te_basesim) {
		templet::actor::engine(e);
		t.engine(te_basesim);
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
        cout << "------ Возврат к предыдущему актору. stage_id текущего = " << stage_id << endl;
        _in->send();
/*$TET$*/
	}

	inline void on_t(templet::basesim_task&t) {
/*$TET$CheckStage$t*/
        t.delay(10); // TODO: поменять потом после проверок
        if (_in->n % saved_number != 0)
        {
            cout << "------ " << _in->n << " не кратно числу " << saved_number << endl;
            out.n = _in->n;
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
	templet::basesim_task t;

/*$TET$CheckStage$$footer*/

    void check_element()
    {
        if (access(_in) && access(out)){
            cout << "------ Проверка элемента последовательности " << _in -> n << " на этапе № " << stage_id << endl;
            if (saved_number == 0)
            {
                saved_number = _in->n;
                cout << "------ Сохранение элемента " << saved_number << " на этапе № " << stage_id << endl;
                
                cout << "---------------------------------"<< endl;
                cout << "|   НАЙДЕНО ПРОСТОЕ ЧИСЛО: " << saved_number << "   |"<< endl;
                cout << "---------------------------------"<< endl;
                
                _in->send();
                
            }
            else
            {
                t.submit();
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
        cout << "--- Сохранение последнего элемента " << last_number << endl;
        
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
/*$TET$*/
