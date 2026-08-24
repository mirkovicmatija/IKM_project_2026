# Parsiranje i vizualizacija CAN prozora pomoću SavvyCAN i CAN DBC datoteka.

## Zadatak i koncept

Zadatak ovog projekta je generisanje CAN prozora pomoću CAN DBC datoteka i grafičko prikazivanje podataka prenesenih korištenjem CAN protokola u programu SavvyCAN.

DBC datoteka predstavlja poseban tip tekstualne baze podataka koja sadrži opis poruke i signala. Opis sadrži informacije o dužini poruke, poziciji podataka, dužini podatka, prvog bita signala, tipu podatka, faktoru skaliranja, offsetu, opsegu i jedinici podatka. 

Jedna DBC datoteka ima naredni format:


Ovaj projektni zadatak se sastoji od koda napisan u programskom jeziku C koji služi za pretvaranje podatka u CAN prozor dekodiranjem izabrane CAN DBC datoteke. Za prikupljanje podataka i komunikaciju koristi se program `candump` iz biblioteke `can-utils`. Za prikaz prikupljenih podataka koristi se program SavvyCAN. 

## Program za pretvaranje sirovih podataka u CAN prozor

