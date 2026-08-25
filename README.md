# Parsiranje i vizualizacija CAN prozora pomoću SavvyCAN i CAN DBC datoteka.

## Zadatak i koncept

Zadatak ovog projekta je generisanje CAN prozora pomoću CAN DBC datoteka i grafičko prikazivanje podataka prenesenih korištenjem CAN protokola u programu SavvyCAN.

DBC datoteka predstavlja poseban tip tekstualne baze podataka koja sadrži opis poruke i signala. Opis sadrži informacije o dužini poruke, poziciji podataka, dužini podatka, prvog bita signala, tipu podatka, faktoru skaliranja, offsetu, opsegu i jedinici podatka. 

Jedna DBC datoteka ima naredni format:


Ovaj projektni zadatak se sastoji od koda napisan u programskom jeziku C koji služi za pretvaranje podatka u CAN prozor dekodiranjem izabrane CAN DBC datoteke. Za prikupljanje podataka i komunikaciju koristi se program `candump` iz biblioteke `can-utils`. Za prikaz prikupljenih podataka koristi se program SavvyCAN. 

## Program za pretvaranje sirovih podataka u CAN prozor

Osnovna ideja za pretvaranje sirovih podatka u CAN prozora je da se na osnovu željene baze podataka koja se proslijeđuje kao argument ovog CLI programa. Prilikom pokretanja programa će se incijalizovati prazna struktura `Array` koja će služiti za čuvanje CAN prozoria i pratećim podacima koje su definisane strukturom `CAN_DBC_Message`. Funkcija:

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
Ova funkcija na osnovu toga da li sistem koristi little endian ili big endian kao način čuvanja podataka u memoriji upisuje podatke na odgovarajuće mjesto.

Sa slanje CAN okvira koristi se SocketCAN infrastruktura.

