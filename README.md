Nume: Chivereanu Radu-Gabriel
Grupă: 335 CA

# Tema 3


## Organizare

1. Solutia implementeaza functiile din fisierul loader.h

- Pentru a retine ce pagini sunt mapate deja in memorie am creat structura segment_info, care 
pentru fiecare segment retine numarul de pagini si daca acestea sunt sau nu mapate.
  
- Tema a fost utila intrucat am pus in practica cunostintele dobandite la curs, astfel asigurandu-ma
ca le-am inteles.

- Implementarea este eficienta. La un moment dat ma gandeam ca identificarea segmentului 
in care se gaseste adresa sa fie facuta cu cautare binara (de aceea am si intrebat pe forum daca adresele virtuale sunt date in ordine crescatoare), insa nu a fost neaparat necesar.

## Implementare

- Intregul enunt al temei e implementat
- Flow-ul este cel cerut in enunt: Interceptez SIGSEGV, lucru ce da trigger functiei de tratare
a semnalului (treat_faulted_address) care identifica segmentul din care face parte adresa faulted si
pagina din cadrul acelui segment, ca mai apoi folosind functia map_page sa incerce sa mapeze bucata in memorie.
- O problema interesanta pe care am depistat-o relativ greu a fost data de faptul ca file_size este unsigned si eu ma asteptam unde calculez cati octeti mai sunt necesari de citit ca un rezultat sa fie 
negativ, lucru care nu se intampla :). Am rezolvat castand la int.

## Cum se compilează și cum se rulează?

- Compilarea se face folosind make.
- Pentru a rula checkerul make & make -f Makefile.checker

## Bibliografie

- Laboratorul 4 SO - Semnale
- Laboratorul 6 SO - Memoria virtuala

## Git

1. https://github.com/Radu1999/elf-loader