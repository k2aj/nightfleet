# Night Fleet

## Dane studenta / projektu

| Temat projektu          | Przedmiot                          |
|-------------------------|------------------------------------|
| Strategiczna gra turowa | Sieci Komputerowe 2 (laboratorium) |

| Nazwisko            | Nr indeksu | Kierunek    | Semestr | Grupa |
|---------------------|------------|-------------|---------|-------|
| Krzysztof Jajeśnica | 145367     | Informatyka | V       | I1.2  |

## Budowanie projektu

### Wymagane oprogramowanie
- CMake ≥3.13
- Dowolny kompilator C++ obsługujący standard C++17 (na pewno działa GCC 11.1.0)
- System operacyjny Linux (z powodu użycia BSD sockets)

### Przygotowanie projektu do budowy
```sh
git clone https://github.com/k2aj/nightfleet
mkdir nightfleet/build && cd nightfleet/build
cmake ..
```
### Budowanie
```sh
cmake --build .
```
Komendę należy uruchomić z utworzonego wcześniej folderu `nightfleet/build`. W wyniku kompilacji powstaną dwa pliki wykonywalne:
- `nfclient` - aplikacja klienta
- `nfserver` - aplikacja serwera