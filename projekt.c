#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>


pthread_t *producenci;
pthread_t *konsumenci;


int *buffor;

sem_t mutex_buffor; 
sem_t pusty; 
sem_t pelny; 

int pozycja_buf=-1; 
int licznik_prod; 
int licznik_kons; 
int dlugosc_buf; 
int licznik; 

int rozmiar = 2, height, width; 
WINDOW *new; 


int produkcja(pthread_t);  
void konsumpcja(int, pthread_t);
void* producent(void*);
void* konsument(void *args);

int main(void){
	
	srand(time(NULL));
	
	initscr();
	cbreak(); 
	 
	start_color(); 
	init_pair(1,COLOR_BLUE,COLOR_BLACK); 
	attron(COLOR_PAIR(1)); 
	
	new = newwin(height - 7,width - 27,12,25); 
	wrefresh(new); 
	box(new,0,0);
	
	wrefresh(new); 
	
	attron(A_UNDERLINE); 
	mvprintw(0,10,"---- PROBLEM PRODUCENTA I KONSUMENTA ----\n");
	attroff(A_UNDERLINE); 
	mvprintw(2,0,"Porgram rozwiazuje problem producenta i konsumenta\nwystepuja dwa procesy: producent i konsument, ktorzy dziela\nwspolny zasob - bufor dla produkowanych jednostek.\nProgram jest wykonany za pomoca semaforow.\nPilnuja one kto w danym momencie ma miec dostep do zasobow.\n");
	attron(A_BOLD);	 
	mvprintw(10,0,"Nacisnij dowolny klawisz, aby kontynuowac!\n");
	attroff(A_BOLD); 
	getch(); 
	clear(); 
	wrefresh(new); 
	
	getmaxyx(stdscr,height,width); 
	new = newwin(height - 1, width - 1, 1, 1); 
	scrollok(new,TRUE); 
	wrefresh(new); 

	sem_init(&mutex_buffor,0,1);
	sem_init(&pelny,0,0);
	
	wrefresh(new); 
	init_pair(1,COLOR_WHITE,COLOR_BLACK); 
	attron(COLOR_PAIR(1)); 
	attron(A_BOLD); 
	refresh();
	mvprintw(1, 5, "Podaj liczbe producentow: ");
	scanw("%d",&licznik_prod);
	
	mvprintw(3, 5, "Podaj liczbe konsumentow: ");
	scanw("%d",&licznik_kons);
	
	mvprintw(5, 5, "Podaj rozmiar buffora: ");
	scanw("%d",&dlugosc_buf);
	
	mvprintw(7, 5, "Podaj ilosc produktow do wytworzenia: ");
	scanw("%d",&licznik);
	attroff(A_BOLD); 
	clear(); 
	wrefresh(new); 
	
	init_pair(1,COLOR_BLUE,COLOR_BLACK); 
	attron(COLOR_PAIR(1)); 
	wbkgd(new, COLOR_PAIR(1)); 


	producenci = (pthread_t*) malloc(licznik_prod*sizeof(pthread_t));
	konsumenci = (pthread_t*) malloc(licznik_kons*sizeof(pthread_t));
	
	buffor = (int*) malloc(dlugosc_buf*sizeof(int));
	sem_init(&pusty,0,dlugosc_buf);

	
	for(int i=0;i<licznik_prod;i++){
		pthread_create(producenci+i,NULL,&producent,NULL);
		refresh(); 
	}

	
	for(int i=0;i<licznik_kons;i++){
		pthread_create(konsumenci+i,NULL,&konsument,NULL);
		refresh(); 
	}

	
	for(int i=0;i<licznik_prod;i++){
		pthread_join(*(producenci+i),NULL);
	}
	for(int i=0;i<licznik_kons;i++){
		pthread_join(*(konsumenci+i),NULL);
	}
	
	clear(); 
	refresh(); 
	init_pair(1,COLOR_WHITE,COLOR_BLACK); 
	attron(COLOR_PAIR(1));
	char exit; 
	
	
	while(exit != 'e') {
	box(new,0,0); 
	attron(A_BOLD); 
	mvprintw(1,3,"Wpisz e aby zakonczyc program: ");
	scanw("%c",&exit);
	clear(); 
	refresh(); 
	if(exit == 'e'){endwin();} 
	}
	return 0; 
}


void* producent(void *args){

	for (int i = 0; i < licznik; i++) {
		
		int produkt = produkcja(pthread_self());
		
		sem_wait(&pusty);
		sem_wait(&mutex_buffor);
		++pozycja_buf; 			
		*(buffor + pozycja_buf) = produkt; 
		sem_post(&mutex_buffor);
		sem_post(&pelny);
		sleep(1);	
	}	
	return NULL;
}



void* konsument(void *args){
	int produkt;
	
	for (int i = licznik; i > 0; i--){
		sem_wait(&pelny);
		sem_wait(&mutex_buffor);
		produkt = *(buffor+pozycja_buf); 
		konsumpcja(produkt,pthread_self());
		--pozycja_buf; 
		sem_post(&mutex_buffor);
		sem_post(&pusty);
		sleep(1); 
	}
	return NULL; 
}

int produkcja(pthread_t self){
	int i = 0;
	int produkt = 1 + rand()%20;
	while(!pthread_equal(*(producenci+i),self) && i < licznik_prod){i++;}
	wrefresh(new); 
	wprintw(new,"Producent %d wytworzyl %d \n",i+1,produkt);++rozmiar;
	wrefresh(new); 
	return produkt; 

void konsumpcja(int produkt,pthread_t self){
	int i = 0;
	attron(A_BOLD); 
	
	while(!pthread_equal(*(konsumenci+i),self) && i < licznik_kons){i++;}
	wrefresh(new); 
	wprintw(new,"Bufor: ");++rozmiar;
	
	wrefresh(new); 
	for(i=0;i<=pozycja_buf;++i){
	 wprintw(new,"%d ",*(buffor+i));++rozmiar;
	 wrefresh(new); 
	}
	attroff(A_BOLD);
	wprintw(new,"\nKonsument %d zuzyl %d \n",i+1,produkt);++rozmiar;
	wrefresh(new); 
}

