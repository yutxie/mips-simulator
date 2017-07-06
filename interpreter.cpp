#include <iostream>
#include <fstream>
#include "interpreter.h"
using namespace std;

interpreter::interpreter(ifstream &_src, istream &_is, ostream &_os) : src(_src), is(_is), os(_os) {
	memset(reg, 0, sizeof reg);
	memset(mem, 0, sizeof mem);
	reg[29] = MAXN - 1;
}

void interpreter::interprete() {
	read_in();
	run();
}

void interpreter::read_in() {
	char str[MAXL];
	int ins_cnt = 0;
	vector<string> name_vec, ph1_vec, ph2_vec, ph3_vec;
	bool text_block = false;
	while (src.getline(str, MAXL, '\n')) {
		string tmp = "";
		int i = 0, l = strlen(str);
		while (str[i] == ' ' || str[i] == '\t') ++i;
		if (str[i] == '.') { // .xxx
			++i;
			tmp = get_phrase(str, i, l);
			if (tmp == "align") {
				++i;
				tmp = get_phrase(str, i, l);
				int n = string_to_int(tmp);
				n = power_2(n);
				heap_top += (n - heap_top  % n) % n;
			}
			else if (tmp == "ascii" || tmp == "asciiz") {
				bool flag = tmp == "asciiz";
				i += 2;
				tmp = get_str(str, i, l - 1);
				for (int i = 0; i < tmp.length(); ++i)
					mem[heap_top++] = tmp[i];
				if (flag) mem[heap_top++] = 0;
			}
			else if (tmp == "byte" || tmp == "half" || tmp == "word") {
				int m = tmp == "byte" ? 1 : (tmp == "half" ? 2 : 4);
				while (true) {
					if (i == l) break;
					++i;
					tmp = get_phrase(str, i, l);
					int n = string_to_int(tmp);
					memcpy(mem + heap_top, &n, m);
					heap_top += m;
				}
			}
			else if (tmp == "space") {
				++i;
				tmp = get_phrase(str, i, l);
				int n = string_to_int(tmp);
				heap_top += n;
			}
			else if (tmp == "data" || tmp == "text") text_block = tmp == "text";
		}
		else if (str[l - 1] == ':') { // label:
			string tmp = get_phrase(str, i, l - 1);
			if (text_block) text_label[tmp] = ins_cnt;
			else data_label[tmp] = heap_top;
		}
		else { // text instruction
			string name = get_phrase(str, i, l); ++i;
			if (name == "") continue;
			++ins_cnt;
			name_vec.push_back(name);
			ph1_vec.push_back(get_phrase(str, i, l)); ++i;
			ph2_vec.push_back(get_phrase(str, i, l)); ++i;
			ph3_vec.push_back(get_phrase(str, i, l)); ++i;
		}
	}
	for (int i = 0; i < ins_cnt; ++i) {
		string name = name_vec[i];
		string ph1 = ph1_vec[i], ph2 = ph2_vec[i], ph3 = ph3_vec[i];
		instruction *ptr = NULL;
		// loading instruction
		if (name == "la") ptr = new la(ph1, ph2);
		if (name == "lb") ptr = new lb(ph1, ph2);
		if (name == "lh") ptr = new lh(ph1, ph2);
		if (name == "lw") ptr = new lw(ph1, ph2);
		// storing instruction
		if (name == "sb") ptr = new sb(ph1, ph2);
		if (name == "sh") ptr = new sh(ph1, ph2);
		if (name == "sw") ptr = new sw(ph1, ph2);
		// assignment instruction
		if (name == "li" || name == "move") ptr = new assignment(ph1, ph2);
		if (name == "mfhi") ptr = new mfhi(ph1);
		if (name == "mflo") ptr = new mflo(ph1);
		// calculation instruction
		if (name == "add") ptr = new add(ph1, ph2, ph3, false);
		if (name == "addu" || name == "addiu") ptr = new add(ph1, ph2, ph3, true);
		if (name == "sub") ptr = new sub(ph1, ph2, ph3, false);
		if (name == "subu") ptr = new sub(ph1, ph2, ph3, true);
		if (name == "mul") ptr = new mul(ph1, ph2, ph3, false);
		if (name == "mulu") ptr = new mul(ph1, ph2, ph3, true);
		if (name == "div") ptr = new __div(ph1, ph2, ph3, false);
		if (name == "divu") ptr = new __div(ph1, ph2, ph3, true);
		if (name == "xor") ptr = new __xor(ph1, ph2, ph3, false);
		if (name == "xoru") ptr = new __xor(ph1, ph2, ph3, true);
		if (name == "neg") ptr = new neg(ph1, ph2, false);
		if (name == "negu") ptr = new neg(ph1, ph2, true);
		if (name == "rem") ptr = new rem(ph1, ph2, ph3, false);
		if (name == "remu") ptr = new rem(ph1, ph2, ph3, true);
		if (name == "seq") ptr = new seq(ph1, ph2, ph3);
		if (name == "sne") ptr = new sne(ph1, ph2, ph3);
		if (name == "sge") ptr = new sge(ph1, ph2, ph3);
		if (name == "sle") ptr = new sle(ph1, ph2, ph3);
		if (name == "sgt") ptr = new sgt(ph1, ph2, ph3);
		if (name == "slt") ptr = new slt(ph1, ph2, ph3);
		// jump instruction
		if (name == "b" || name == "j" || name == "jr") ptr = new jump(ph1);
		if (name == "jal" || name == "jalr") ptr = new jal(ph1);
		if (name == "beq") ptr = new beq(ph1, ph2, ph3);
		if (name == "bne") ptr = new bne(ph1, ph2, ph3);
		if (name == "bge") ptr = new bge(ph1, ph2, ph3);
		if (name == "ble") ptr = new ble(ph1, ph2, ph3);
		if (name == "bgt") ptr = new bgt(ph1, ph2, ph3);
		if (name == "blt") ptr = new blt(ph1, ph2, ph3);
		if (name == "beqz") ptr = new beq(ph1, "0", ph2);
		if (name == "bnez") ptr = new bne(ph1, "0", ph2);
		if (name == "bgez") ptr = new bge(ph1, "0", ph2);
		if (name == "blez") ptr = new ble(ph1, "0", ph2);
		if (name == "bgtz") ptr = new bgt(ph1, "0", ph2);
		if (name == "bltz") ptr = new blt(ph1, "0", ph2);
		// syscall instruction
		if (name == "syscall") ptr = new syscall(is, os);
		if (name == "nop") ptr = new instruction();
		
		ptr->line = i;
		ins_vec.push_back(ptr);
	}
}

void interpreter::write_back() {
	unique_lock<mutex> lock(mtx);
	while (rep[3].empty()) rep_empty[3].wait(lock);
	instruction* ptr = rep[3].front();
	rep[3].pop_front();

	ptr->write_back();
	jump_cnt -= ptr->jump_type;
	if (jump_cnt == 0) jum.notify_all();
	auto it = ptr->reg_to_write.begin();
	while (it != ptr->reg_to_write.end()) --reg_cnt[*(it++)];
}

void interpreter::memory_access() {
	unique_lock<mutex> lock(mtx);
	while (rep[2].empty()) rep_empty[2].wait(lock);
	instruction* ptr = rep[2].front();
	rep[2].pop_front();
	
	ptr->memory_access();

	rep[3].push_back(ptr);
	rep_empty[3].notify_all();
}

void interpreter::execute() {
	unique_lock<mutex> lock(mtx);
	while (rep[1].empty()) rep_empty[1].wait(lock);
	instruction* ptr = rep[1].front();
	rep[1].pop_front();
	
	ptr->execute();

	rep[2].push_back(ptr);
	rep_empty[2].notify_all();
}

void interpreter::data_prepare() {
	unique_lock<mutex> lock(mtx);
	while (rep[0].empty()) rep_empty[0].wait(lock);
	instruction* ptr = rep[0].front();
	rep[0].pop_front();
	
	ptr->data_prepare();

	rep[1].push_back(ptr);
	rep_empty[1].notify_all();
}

void interpreter::instruction_fetch() {
	unique_lock<mutex> lock(mtx);
	while (jump_cnt > 0) jum.wait(lock);

	instruction *ptr = ins_vec[ins_top];
	bool reg_conflict = false;
	auto it = ptr->reg_to_read.begin();
	while (it != ptr->reg_to_read.end())
		if (reg_cnt[*(it++)] > 0) reg_conflict = true;
	if (reg_conflict) ptr = NULL;
	else {
		jump_cnt += ptr->jump_type;
		it = ptr->reg_to_write.begin();
		while (it != ptr->reg_to_write.end()) ++reg_cnt[*(it++)];
		++ins_top;
	}

	rep[0].push_back(ptr);
	rep_empty[1].notify_all();
}

void interpreter::run() {
	jump_cnt = 0;
	ins_vec_sz = ins_vec.size();
	ins_top = text_label["main"];
	memset(reg_cnt, 0, sizeof reg_cnt);

	while (true) {
		thread t[5];
		t[0] = thread(bind(&interpreter::write_back, this));
		t[1] = thread(bind(&interpreter::memory_access, this));
		t[2] = thread(bind(&interpreter::execute, this));
		t[3] = thread(bind(&interpreter::data_prepare, this));
		t[4] = thread(bind(&interpreter::instruction_fetch, this));
		for (int i = 0; i < 5; ++i) t[i].join();
	}
}