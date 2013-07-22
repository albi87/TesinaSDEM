/**
	Progetto di Laboratorio di Ingegneria Informatica
	Sistemi di Elaborazione Multimediale

	Titolo: Progetto e implementazione di un software per la gestione 
			archivi di file compressi con LZ77 ottimizzato in termini di velocità

	Leader di progetto:	Paganelli Alberto
	Altri componenti:   Montanari Mirko
						Lancellotti Matteo
**/

#include <iostream>
#include <intrin.h>
#include <time.h>
#include <vector>
#include <string>
#include "getopt.h"
#include "dirent.h"
#include "lzss.h"
#include "huffman.h"
#include "hash.h"

using namespace std;

static void guida(){
	cout << endl << "Sintassi corretta: LMPzip [-c|-d] -i file_sorgente [-o file_destinazione] [-B valore]|[-O valore]|[-D valore] [-r] [-a|-b] [-h]" << endl << endl;
	cout << "-c \t compressione del file" << endl;
	cout << "-d \t decompressione del file" << endl;
	cout << "-i \t nome del file sorgente" << endl;
	cout << "-o \t nome file destinazione" << endl;
	cout << "-B \t dimensione del buffer (default: 4096)" << endl;
	cout << "-O \t dimensione del lookhead (default: 32)" << endl;
	cout << "-D \t dimensione dizionario hash (default: 10)" << endl;
	cout << "-r \t cartella" << endl;
	cout << "-a \t funzione hash classica" << endl;
	cout << "-b \t funzione hash gzip" << endl;
	cout <<"-h \t guida all'uso" << endl << endl;
	return;
}

void myfunc(string s){}

void main(int argc, char *argv[]){

	clock_t start, end;
	double tempo;
	int ch;
	unsigned buff = 4096;
	unsigned ov = 32;
	unsigned diz = 10;
	string input_file;
	string output_file;
	bool primo = true;
	bool compr = true;
	bool ricorsivo = false;
	vector<string> files;
	start = clock();

	while ((ch = getopt(argc, argv, "r:cdi:o:B:O:D:abh")) != -1) {
		switch (ch) {
			case 'c':
				compr = true;
				break;
			case 'd':
				compr = false;
				break;
			case 'i':
				input_file = optarg;
				break;
			case 'o':
				output_file = optarg;
				break;
			case 'B':
				buff = atoi(optarg);
				if(buff == 0){
					guida();
					return;
				}
				break;
			case 'O':
				ov = atoi(optarg);
				if(ov == 0){
					guida();
					return;
				}
				break;
			case 'D':
				diz = atoi(optarg);
				if(diz == 0){
					guida();
					return;
				}
				break;
			case 'r':
				ricorsivo = true;
				DIR* d;
				input_file = optarg;
				d = opendir(input_file.c_str());
				if(d)
				{
					dirent *f;
					while (f = readdir(d))
					{
						if(f->d_type == 32768)
						{
							files.push_back(f->d_name);
						}
					}
				}
				else
				{
					cout<<"Hai inserito un percorso sbagliato!";
					guida();
					return;
				}
				break;
			case 'a':
				primo = true;
				break;
			case 'b':
				primo = false;
				break;
			case 'h':
			default:
				guida();
				return;

		}
	}

	if(compr == true && (input_file.length()==0 || output_file.length()==0)){
		guida();
		return;
	}

	if(compr == false && input_file.length() == 0){
		guida();
		return;
	}

	cout << endl;
	cout << "Progetto di Laboratorio di Ingegneria Informatica" << endl;
	cout << "Sistemi di Elaborazione Multimediale" << endl;
	cout << "Titolo: Progetto e implementazione di un software per" << endl;
	cout << "\t la gestione di archivi di file compressi con LZ77 ottimizzato in termini di velocita'" << endl << endl;
	cout << "Leader di progetto: Paganelli Alberto" << endl;
	cout << "Altri componenti: Montanari Mirko e Lancellotti Matteo" << endl << endl;
	
	//funzione di compressione
	if(compr == true){
		
		cout << "Compressione in corso..." << endl;
		fstream fout(output_file, fstream::out | fstream::binary);		

		if(ricorsivo)
		{
			//for_each(files.begin(), files.end(), myfunc);
			unsigned num_files = files.size();
			fout.write((char*)&num_files,sizeof(char));
			for(unsigned i = 0; i< num_files; ++i)
			{
				//compressione
				if(primo == true){
					LMPhash h(diz,ov,buff);
					h.creaFileHash(input_file+"\\"+files[i],"hash.tmp");
				}else
				{
					Lzss_lmp l(buff, ov);
					l.lzss_codifica(input_file+"\\"+files[i],"hash.tmp");
				}
				fout << files[i];
				fout << "#";
				unsigned long n = 100;
				fout.write((char*)&n,sizeof(unsigned long));
				huffman huff("hash.tmp");
				huff.calcolaHuffman();
				n = huff.scriviHuffman(fout);
				long pos = fout.tellp();
				fout.seekp(pos - n - 4);
				fout.write((char*)&n,sizeof(long));
				fout.seekp(pos);
				//cancella il file temporaneo
				_unlink("hash.tmp");
			}

			fout.close();
		}
		else
		{
			//compressione
			unsigned num_files = 1;
			fout.write((char*)&num_files,sizeof(char));
			if(primo == true){
				LMPhash h(diz,ov,buff);
				h.creaFileHash(input_file,"hash.tmp");
			}else
			{
				Lzss_lmp l(buff, ov);
				l.lzss_codifica(input_file,"hash.tmp");
			}

			fout << argv[3];
			fout << "#";
			unsigned long n = 100;
			fout.write((char*)&n,sizeof(unsigned long));
			//fout << "#";
			huffman huff("hash.tmp");
			huff.calcolaHuffman();
			n = huff.scriviHuffman(fout);
			long pos = fout.tellp();
			fout.seekp(pos - n - 4);
			fout.write((char*)&n,sizeof(long));

			fout.close();
			//cancella il file temporaneo
			_unlink("hash.tmp");
		}

		cout << "Compressione eseguita!" << endl << endl;
	}

	//funzione di decompressione
	if(compr == false){
		cout << "Decompressione in corso..." << endl;
		//decompressione
		fstream fin(input_file, fstream::in | fstream::binary);
		unsigned num_files = 0;
		fin.read((char*)&num_files, sizeof(char));
		for (unsigned i = 0; i < num_files; ++i)
		{
			huffman huf("hash2.tmp");
			string nome = huf.leggiHuffman(fin);
			Lzss_lmp l;
			l.lzss_decodifica("hash2.tmp", nome);
			//lzss_decodifica("hash.tmp", "test.pdf");
			_unlink("hash2.tmp");
		}
		
		cout << "Decompressione eseguita!" << endl << endl;
	}
	
	end = clock();
	tempo = ((double)(end-start)/CLOCKS_PER_SEC);

	cout << "Tempo impegato: " << tempo << endl;

}


