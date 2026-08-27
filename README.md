# Parsiranje i vizualizacija CAN okvira pomoću SavvyCAN-a i CAN DBC datoteka

## Zadatak i koncept

Zadatak ovog projekta je generisanje CAN okvira pomoću CAN DBC datoteka i grafičko prikazivanje podataka prenesenih korištenjem CAN protokola u programu SavvyCAN.

DBC datoteka predstavlja poseban tip tekstualne baze podataka koja sadrži opise poruka i signala. Opis sadrži informacije o dužini poruke, poziciji podataka, dužini signala, početnom bitu signala, tipu podatka, faktoru skaliranja, offsetu, opsegu i mjernoj jedinici podatka.

Jedna DBC datoteka može sadržavati zapis sljedećeg oblika:

```text
BO_ 371 Naziv_poruke: 8 Vector__XXX
   SG_ Naziv_signala : 0|16@1+ (1,1000) [0|1000] "mV" Vector__XXX
```

Oznaka `BO_` predstavlja početak definicije poruke. Broj `371` predstavlja identifikator CAN poruke, `Naziv_poruke` predstavlja naziv poruke, a broj `8` predstavlja dužinu poruke u bajtovima. Oznaka `SG_` predstavlja definiciju signala, nakon koje slijedi naziv signala. Forma `0|16@1+` označava da signal počinje od bita 0, da je dužine 16 bita, da koristi little-endian raspored bajtova i da je neoznačen. U formi `(1,1000)` prvi broj označava faktor skaliranja, a drugi offset. Fizička vrijednost dobija se primjenom faktora i offseta na sirovu vrijednost signala. Vrijednosti u uglastim zagradama predstavljaju opseg fizičkih vrijednosti, dok string pod navodnicima predstavlja mjernu jedinicu.

Ovaj projektni zadatak sastoji se od koda napisanog u programskom jeziku C, koji služi za formiranje CAN okvira na osnovu izabrane CAN DBC datoteke i unesenih fizičkih vrijednosti signala. Za prikupljanje podataka i komunikaciju koristi se program `candump` iz paketa `can-utils`. Za prikaz prikupljenih podataka koristi se program SavvyCAN.

## Program za pretvaranje sirovih podataka u CAN okvir

Izvorni kod nalazi se u direktoriju `src`. Direktorij sadrži tri datoteke, od kojih je jedna zaglavlje. U zaglavlju `src/dbc.h` deklarisane su strukture podataka i funkcije neophodne za rad programa. Datoteka `src/dbc.c` sadrži tijela funkcija deklarisanih u zaglavlju. Datoteka `src/can_generator.c` sadrži funkciju `main()`, u kojoj se pozivaju funkcije za formiranje CAN okvira i slanje CAN poruka.

Osnovna ideja programa je da, na osnovu željene CAN DBC baze podataka koja se prosljeđuje kao argument CLI programa, formira CAN okvire iz unesenih fizičkih vrijednosti. Prilikom pokretanja programa inicijalizuje se prazna struktura `Array`, koja služi za čuvanje CAN poruka i pratećih podataka definisanih strukturom `CAN_DBC_Message`. Ove strukture definisane su u zaglavlju `src/dbc.h`. Funkcija:

```c
void dbcParser(const char* filename, struct Array *array);
```

uzima kao argumente relativnu putanju do CAN DBC baze podataka i ranije definisanu praznu listu. Ako je putanja pogrešna ili datoteka ne postoji, program će ispisati informaciju o grešci i prekinuti dalje izvršavanje. Ako je putanja validna, izvršiće se detaljno iščitavanje CAN DBC datoteke. Ako se detektuje linija `BO_`, u listu će se dodati nova prazna poruka sa odgovarajućim identifikatorom. Ako se detektuje linija `SG_`, biće omogućen unos podatka koji se šalje i biće učitani atributi signala iz CAN DBC baze podataka. Na osnovu tih atributa podatak će se formirati i upisati u CAN okvir. Funkcija koja se koristi za pretvaranje fizičke vrijednosti u sirovu CAN vrijednost je oblika:

```c
unsigned int getCANdataFromPhysical(int physicalValue, double factor, double offset) {
    return (unsigned int)((physicalValue - offset) / factor);
}
```

Za upis podataka u CAN okvir koristi se funkcija oblika:

```c
void insertSignalIntoMessage(unsigned char *message, unsigned int start_bit, unsigned int signal_length, unsigned int signal_value, const unsigned short endian)
```

Ova funkcija upisuje podatke u CAN okvir u skladu sa redoslijedom bajtova (little endian ili big endian) definisanim za signal.

Za slanje CAN okvira koristi se SocketCAN infrastruktura. Da bi se omogućilo slanje, potrebno je kreirati objekat `can_frame` i socket te podesiti odgovarajuća polja, uključujući `can_family` i `can_ifindex`. Funkcijom:

```c
bind(s, (struct sockaddr *)&addr, sizeof(addr));
```

povezuje se socket sa CAN interfejsom i omogućava komunikacija putem CAN busa.

Pomoću `for` petlje pristupa se članovima liste `dbcArray`, nakon čega se objektu `can_frame` dodjeljuju identifikator, dužina poruke i podaci CAN okvira. Poruka se zatim šalje pomoću funkcije:

```c
write(s, &frame, sizeof(frame));
```

Na kraju se zatvara socket i poziva funkcija za oslobađanje liste `dbcArray`.

## Uputstvo za izvršavanje koda

Za rad programa potrebno je koristiti dva terminala. U prvom terminalu, nakon preuzimanja koda, potrebno je unijeti komandu:

```bash
make all
```

Ova komanda služi za kompajliranje programa. Izvršna datoteka naziva se `./can_generator`.

Nakon kompajliranja koda potrebno je aktivirati CAN interfejs komandom:

```bash
sudo ip link set can0 up type can bitrate 125000
```

U drugom terminalu potrebno je pokrenuti program `candump` iz paketa `can-utils`, koji služi za prikupljanje podataka poslanih putem CAN busa. U terminal je potrebno unijeti komandu:

```bash
candump -L can0 > obd_candump.log
```

Ovom komandom pokreće se `candump` na interfejsu `can0` i omogućava upis poslanih CAN okvira u datoteku `obd_candump.log`.

U prvom terminalu pokreće se program `./can_generator`:

```bash
./can_generator "putanja/do/željene/datoteke.dbc"
```

Pokretanjem programa pojavljuje se linija za unos fizičke vrijednosti signala koji će se poslati. Nakon što se unesu sve vrijednosti za sve poruke definisane CAN DBC datotekom, u terminalu će se ispisati svi okviri koji će biti proslijeđeni.

Nakon završenog unosa podataka potrebno je prebaciti datoteku `obd_candump.log` pomoću komande `scp` sa Raspberry Pi platforme na uređaj na kojem je instaliran program SavvyCAN. Nakon otvaranja programa SavvyCAN potrebno je izabrati opciju `DBC File Manager` i učitati CAN DBC datoteku koja je korištena za formiranje okvira. Nakon učitavanja DBC datoteke potrebno je otvoriti prozor `Playback` i učitati log datoteku.

Video tutorijal može se pronaći na sljedećem linku: [SavvyCAN tutorijal](https://youtu.be/PLyD2AcmKeA)
