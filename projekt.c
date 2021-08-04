#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>

//wskaźnik na wątki producentów
pthread_t *producenci;
//wskaźnik na wątki konsumentów
pthread_t *konsumenci;

//przechowuje elementy buffora
int *buffor;

sem_t mutex_buffor; //dopuszcza tylko jedn zasob w jedym momencie
sem_t pusty; // rozmiar bufora
sem_t pelny; // poziom zapełnienia bufora

int pozycja_buf=-1; // pozycja bufora -1, bo jak zacznie działać to zeby miał wartość 0 na początku
int licznik_prod; // liczba producentow
int licznik_kons; // liczba konsumentow
int dlugosc_buf; // dlugosc bufora
int licznik; // ilosc produktow do wytworzenia

int rozmiar = 2, height, width; //rozmiar okienka
WINDOW *new; //stworzenie nowego okna

//deklaracja funkcji produkującej produkty
int produkcja(pthread_t);  
//deklaracja funkcji zużywającej produkty
void konsumpcja(int, pthread_t);
//deklaracja funkcji producent
void* producent(void*);
//deklaracja funkcji konsumenta
void* konsument(void *args);

int main(void){
	//losowanie randomowego czasu
	srand(time(NULL));
	
	initscr(); // inicjalizacja okna 
	cbreak(); // wyłacza bufforowanie linii
	 
	start_color(); //uruchamia mozliwosc zmiany kolorow
	init_pair(1,COLOR_BLUE,COLOR_BLACK); //wybranie jaka ma byc para kolorow
	attron(COLOR_PAIR(1)); //uruchomienie kolorow
	
	new = newwin(height - 7,width - 27,12,25); // wartosci poczatkowe dla okna
	wrefresh(new); // odswiezenie
	box(new,0,0);	// strorzenie okna
	
	wrefresh(new); //odświezenie
	
	attron(A_UNDERLINE); //wlaczenie podkreslenia tekstu
	mvprintw(0,10,"---- PROBLEM PRODUCENTA I KONSUMENTA ----\n");
	attroff(A_UNDERLINE); //wylaczenie podkreslenia tekstu
	mvprintw(2,0,"Porgram rozwiazuje problem producenta i konsumenta\nwystepuja dwa procesy: producent i konsument, ktorzy dziela\nwspolny zasob - bufor dla produkowanych jednostek.\nProgram jest wykonany za pomoca semaforow.\nPilnuja one kto w danym momencie ma miec dostep do zasobow.\n");
	attron(A_BOLD);	 //wlaczenie pogrubienia
	mvprintw(10,0,"Nacisnij dowolny klawisz, aby kontynuowac!\n");
	attroff(A_BOLD); //wylaczenie pogrubienia
	getch(); //oczekiwanie na wcisniece klawisza
	clear(); //wyczyszczenie ekranu
	wrefresh(new); //odświezenie
	
	getmaxyx(stdscr,height,width); //pobieranie wartosci podstawowych okna
	new = newwin(height - 1, width - 1, 1, 1); //nadanie parametrow startowych okna
	scrollok(new,TRUE); //uruchomienie mozliwosci scrollowania sie obrazu
	wrefresh(new); //odświezenie
	
	//inicjalizacja semaforow mutex i zapenienia
	sem_init(&mutex_buffor,0,1);
	sem_init(&pelny,0,0);
	
	wrefresh(new); //odświezenie
	init_pair(1,COLOR_WHITE,COLOR_BLACK); //wybranie jaka ma byc para kolorow
	attron(COLOR_PAIR(1)); //wlaczenie pary kolorow
	attron(A_BOLD); //wlaczenie podrubienia
	refresh(); //odświezenie
	//wprowadzenie liczby producentow
	mvprintw(1, 5, "Podaj liczbe producentow: ");
	scanw("%d",&licznik_prod);
	//wprowadzenie liczby konsumentow
	mvprintw(3, 5, "Podaj liczbe konsumentow: ");
	scanw("%d",&licznik_kons);
	//wprowadzenie rozmiaru buffora
	mvprintw(5, 5, "Podaj rozmiar buffora: ");
	scanw("%d",&dlugosc_buf);
	//wprowadzenie ilosci produktow do wytworzenia
	mvprintw(7, 5, "Podaj ilosc produktow do wytworzenia: ");
	scanw("%d",&licznik);
	attroff(A_BOLD); //wylaczenie pogrubienia
	clear(); //wyczyszczenie ekranu
	wrefresh(new); //odświezenie
	
	init_pair(1,COLOR_BLUE,COLOR_BLACK); //wybranie par kolorow
	attron(COLOR_PAIR(1)); //uruchomienie par kolorow
	wbkgd(new, COLOR_PAIR(1)); //zastosowanie kolorow dla calego okna

	//dynamiczna alokacja pamieci dla watkow
	producenci = (pthread_t*) malloc(licznik_prod*sizeof(pthread_t));
	konsumenci = (pthread_t*) malloc(licznik_kons*sizeof(pthread_t));
	//dynamiczna alokacja pamieci dla buffora
	buffor = (int*) malloc(dlugosc_buf*sizeof(int));
	sem_init(&pusty,0,dlugosc_buf);

	// kreowanie watkow producentow
	for(int i=0;i<licznik_prod;i++){
		pthread_create(producenci+i,NULL,&producent,NULL);
		refresh(); //odświezenie
	}

	// kreowanie watkow konsumentow
	for(int i=0;i<licznik_kons;i++){
		pthread_create(konsumenci+i,NULL,&konsument,NULL);
		refresh(); //odświezenie
	}

	//oczekiwanie na zakonczenie watku
	for(int i=0;i<licznik_prod;i++){
		pthread_join(*(producenci+i),NULL);
	}
	for(int i=0;i<licznik_kons;i++){
		pthread_join(*(konsumenci+i),NULL);
	}
	
	clear(); //wyczyszczenie ekranu
	refresh(); //odświezenie
	init_pair(1,COLOR_WHITE,COLOR_BLACK); //wybranie par kolorow
	attron(COLOR_PAIR(1)); //wlaczenie pary kolorow
	char exit; //zmienna przechowujaca znak zakonczenia programu
	
	//zakonczenie programu
	while(exit != 'e') { //warunek za trzeba wcisnac "e" aby wyjsc z programu
	box(new,0,0); //utworzenie okna
	attron(A_BOLD); //wlaczenie pogrubienia
	//wczytanie znaku
	mvprintw(1,3,"Wpisz e aby zakonczyc program: ");
	scanw("%c",&exit);
	clear(); //wyczyszczenie ekranu
	refresh(); //odświezenie
	if(exit == 'e'){endwin();} //jesli prawidlowy znak to zakmniecie okna
	}
	return 0; //zakonczenie programu
}

/* w sekcji krytycznej moze wykonywac sie tylko jeden watek dlatego uzywamy mutex aby pilnowal kto wszedl
   do sekcji krytycznej. Mutex jest specjalnym rodzajem semafora, poniewaz moze go odblokowac i zablokowac tylko ten
   watek, który go wytworzyl
*/ 

//funkcja producenta
void* producent(void *args){

	//petla wywolujaca produkcje produktow
	for (int i = 0; i < licznik; i++) {
		// produkowany produkt
		int produkt = produkcja(pthread_self());
		// jesli nie ma wolnego miejsca to czeka
		sem_wait(&pusty);
		// jesli inny watek korzysta z buffora to czeka
		//wejscie do sekcji krytycznej
		sem_wait(&mutex_buffor);
		++pozycja_buf; //zmienia pozycje buffora			
		*(buffor + pozycja_buf) = produkt; //dodanie produktu do buffora
		// odblokowuuje dostep do bufora
		sem_post(&mutex_buffor);
		//wyjscie z sekcji krytycznej
		// zwieksza liczbe pelnych slotow
		sem_post(&pelny);
		sleep(1); //uspienie, aby producent i konsument dzialali na zmiane	
	}	
	return NULL; //zwrocenie zamiast zera braku wartosci
}

/* w sekcji krytycznej moze wykonywac sie tylko jeden watek dlatego uzywamy mutex aby pilnowal kto wszedl
   do sekcji krytycznej. Mutex jest specjalnym rodzajem semafora, poniewaz moze go odblokowac i zablokowac tylko ten
   watek, który go wytworzyl
*/ 

//funkcja konsumenta
void* konsument(void *args){
	int produkt;
	//petla wywolujaca zuzywanie produktow
	for (int i = licznik; i > 0; i--){
		// jesli jest pusty to czeka
		sem_wait(&pelny);
		// jesli inny watek korzysta z buffora to czeka
		//wejscie do sekcji krytycznej
		sem_wait(&mutex_buffor);
		produkt = *(buffor+pozycja_buf); //pobranie produktu z bufora
		// zuzywa produkt
		konsumpcja(produkt,pthread_self());
		--pozycja_buf; //zmienia pozycje buffora
		// odblokowuuje dostep do bufora
		sem_post(&mutex_buffor);
		//wyjscie z sekcji krytycznej
		// zwieksza liczbe pelnych slotow
		sem_post(&pusty);
		sleep(1); //uspienie, aby producent i konsument dzialali na zmiane	
	}
	return NULL; //zwrocenie zamiast zera braku wartosci
}

int produkcja(pthread_t self){
	int i = 0;
	//int produkt;
	int produkt = 1 + rand()%20;
	//sprawdza ktory watek producentama byc wykonany w danym momencie
	while(!pthread_equal(*(producenci+i),self) && i < licznik_prod){i++;}
	wrefresh(new); //odswiezenie okna
	wprintw(new,"Producent %d wytworzyl %d \n",i+1,produkt);++rozmiar;
	wrefresh(new); //odswiezenie okna
	return produkt; //zwrocenie numeru produktu jako int (calkowita wartosc)
}

void konsumpcja(int produkt,pthread_t self){
	int i = 0;
	attron(A_BOLD); //wlaczenie pogrubienia
	//sprawdza ktory watek konsumenta ma byc wykonany w danym momencie
	while(!pthread_equal(*(konsumenci+i),self) && i < licznik_kons){i++;}
	wrefresh(new); //odswiezenie okna
	wprintw(new,"Bufor: ");++rozmiar;
	//wypisanie jakie elementy znajduja sie w buforze
	wrefresh(new); //odswiezenie okna
	for(i=0;i<=pozycja_buf;++i){
	 wprintw(new,"%d ",*(buffor+i));++rozmiar; //wypisuje kolejne elementy z buffora
	 wrefresh(new); //odswiezenie okna
	}
	attroff(A_BOLD); //wylaczenie pogrubienia
	wprintw(new,"\nKonsument %d zuzyl %d \n",i+1,produkt);++rozmiar;
	wrefresh(new); //odswiezenie okna
}

