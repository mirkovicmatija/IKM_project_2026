# Parsiranje i vizualizacija CAN prozora pomoću SavvyCAN i CAN DBC datoteka.

## Zadatak i koncept

Zadatak ovog projekta je generisanje CAN prozora pomoću CAN DBC datoteka i grafičko prikazivanje podataka prenesenih korištenjem CAN protokola u programu SavvyCAN.

DBC datoteka predstavlja poseban tip tekstualne baze podataka koja sadrži opis poruke i signala. Opis sadrži informacije o dužini poruke, poziciji podataka, dužini podatka, prvog bita signala, tipu podatka, faktoru skaliranja, offsetu, opsegu i jedinici podatka. 

Jedna DBC datoteka ima naredni format:
```text
BO_ 371 Naziv_poruke: 8 Vector__XXX
   SG_ Naziv_signala : 0|16@1+ (1,1000) [0|1000] "mV" Vector__XXX
```
gdje `BO_` predstavlja oznaku početka poruke, naredni broj (`371`) predstavlja identifikator CAN poruke, naredni string predstavlja ime poruke (`Naziv_poruke`), a broj predstavlja broj signala u porukama. `SG_` je identifikator signala kojeg prati naziv signala. Forma `0|16@1+` označava da poruka počinje od prvog bit-a, dužine je 16 bita, upisana kao little endian i neoznačena je. U formi `(1,1000)` prvi broj označava faktor sa kojim se množi, a drugi offset koji se dodaje na CAN podatak kako bi dobili fizičku vrijednost. Naredna dva broja predstavljaju opseg vrijednosti dok string pod navodnicima mjernu jedinicu.   


Ovaj projektni zadatak se sastoji od koda napisan u programskom jeziku C koji služi za pretvaranje podatka u CAN prozor dekodiranjem izabrane CAN DBC datoteke. Za prikupljanje podataka i komunikaciju koristi se program `candump` iz biblioteke `can-utils`. Za prikaz prikupljenih podataka koristi se program SavvyCAN. 

## Program za pretvaranje sirovih podataka u CAN prozor

Izvorni kod se nalazi u fascikli `src`. Fascikla sadrži 3 datoteke od koje je jedna zaglavlje. U zaglavlju, datoteka `src/dbc.h`, su deklarisane strukture podataka i funkcije neophodne za rad programa. Datoteka `src/dbc.c` sadrži tijela funkcija deklarisanih u zaglavlju. Datoteka `src/can_generator.c` sadrži `main()` funkciju u kojoj se poziva funkcija za formiranje CAN okvira i pozivaju se potrebne funkcije za slanje CAN poruke.        

Osnovna ideja za pretvaranje sirovih podatka u CAN prozora je da se na osnovu željene baze podataka koja se proslijeđuje kao argument ovog CLI programa. Prilikom pokretanja programa će se incijalizovati prazna struktura `Array` koja će služiti za čuvanje CAN prozoria i pratećim podacima koje su definisane strukturom `CAN_DBC_Message`. Ove strukture su definisane u zaglavlju (`src/dbc.h`). Funkcija:

```c
void dbcParser(const char* filename, struct Array *array);
``` 

uzima kao argumente relativnu adresu CAN DBC baze podataka i ranije definisanu praznu listu. U slučaju da je pogrešna adresa ili ako je adresa nepostojeća program će ispisati informaciju o grešci i prestati sa daljim izvršavanjem. Ako je adresa validna izvršiće se detaljno iščitavanje CAN DBC datoteke. U slučaju da se detektuje linija `BO_` CAN DBC datoteke dodaće se nova prazna poruka u listu sa odgovarajućim identifikatorom. Ako se detektuje linija `SG_`, biće omogućen unos podatka koji se šalje i biće učitani atributi signala iz CAN DBC baze podataka. Na osnovu atributa formiraće se i upisati podatak u CAN prozor. Funkcija koja se koristi za pretvaranje fizičke vrijednosti u CAN okvir je oblika:

```c
unsigned int getCANdataFromPhysical(int physicalValue, double factor, double offset) {
    return (unsigned int)((physicalValue - offset)/ factor);
}
```
Za upis podataka u CAN okvir koristi se funkcija oblika:

```c 
void insertSignalIntoMessage(unsigned char *message, unsigned int start_bit, unsigned int signal_length, unsigned int signal_value, const unsigned short endian) 
```
Ova funkcija upisuje podatke na osnovu toga da li sistem koristi little endian ili big endian kao način čuvanja podataka u memoriji.

Za slanje CAN okvira koristi se SocketCAN infrastruktura. Kako bi omogućili slanje potrebno je da kreirati `can_frame` objekat i socket i namjestiti polja za `can_family` i `can_ifindex`. Funkcijom: 
```c bind(s, (struct sockaddr *)&addr, sizeof(addr));```
povezujemo socket i omogućavamo da se komunikacija CAN-BUS protokolom može izvršiti.

Pomoću for petlje pristupamo članovima liste `dbcArray`, dodjeljujemo vrijednosti za identifikator, dužinu poruke i CAN prozor `can_frame` objektu i šaljemo poruku pomoću funkcije: 
```c  write(s, &frame, sizeof(frame));```  

Na kraju zatvaramo konekciju i pozivamo funkciju za oslobađanje liste `dbcArray`.  

## Uputstvo za izvršavanje koda

Za rad programa potrebno je koristiti dva terminala. U prvom terminalu nakon preuzimanja koda potrebno je pokrenuti u terminal unijeti komandu: 
```bash
make all
```
koja služi za kompajliranje programa. Izvršna datoteka se naziva `./can_generator`.
Nakon kompajliranja koda potrebno je uključiti CAN interfejs komandom:
```bash
sudo ip link set can0 up type can bitrate 125000 
```

U drugom terminalu potrebno je unutar datoteke `can-utils` ili neke druge datoteke koja sadrži `candump` program, koji služi za prikupljanje podataka poslanih CAN-BUS protokolom. Potrebno je u liniju terminala unijeti linuju: 
```bash
candump -L can0 > obd_candump.log
```
kojom pokrećemo `candump` na `can0` interfejsu i omogućavamo upis poslanih CAN okvira u `obd_candump.log`.

U prvom terminalu pokreće se program `./can_generator`.

```bash
./can_generator "putanja/do/željene/datoteke.dbc"
```
Pokretanjem koda pojavljuje se linija za unos fizičke vrijednosti signala koji će se poslati. Nakon što se unesu sve vrijednosti za sve poruke koje su definisane CAN DBC datotekom, u terminalu će se ispisati svi okviri koji će se proslijediti. 

Nakon završenog unosa podataka potrebno je prebaciti `obd_candump.log` datoteku pomoću komande `scp` sa Raspberry Pi platfomre na uređaj sa intaliranim SavvyCAN programom. Nakon otvaranja programa SavvyCAN potrebno je izabrati opciju `DBC File Manager` i potrebno je unijeti CAN DBC datoteku koji je korišten za pretvaranje. Nakon unosa DBC CAN datoteke potrebno je otvoriti prozor `Playback` i učitati log datoteku.

Video tutorijal može se pronaći na linku: https://youtu.be/PLyD2AcmKeA