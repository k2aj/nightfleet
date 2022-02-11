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
- GLFW3
- System operacyjny Linux (z powodu użycia BSD sockets)
- OpenGL ≥4.3 (tylko do uruchomienia klienta)

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

## Struktura projektu
- `nfclient` (`source/client`) - **aplikacja klienta**
  - `dgl/`, `graphics.cpp` - renderowanie za pomocą OpenGL
- `nfserver` (`source/server/`) - **aplikacja serwera**
  - `connectionhandler.cpp` - wątek obsługujący klienta
  - `gamemangager.cpp` - tworzenie rozgrywek i przydzielanie do nich graczy
  - `usermanager.cpp` - logowanie użytkowników do systemu
- `nfcommon` (`source/common/`) - **biblioteka zawierająca kod wspólny dla klienta i serwera**
  - `engine/` - logika wewnętrzna gry
  - `network/`, w szczególności `network/protocol.cpp` - kod sieciowy
- W folderze `libraries` znajduje się kod źródłowy wykorzystanych bibliotek zewnętrznych

### Wykorzystane biblioteki
- [GLFW](https://github.com/glfw/glfw/) (tworzenie okien)
- GLAD (loader OpenGL; wygenerowano z [tej strony](https://glad.dav1d.de/))
- [GLM](https://github.com/g-truc/glm/) (obliczenia wektorowo/macierzowe)
- [Dear ImGui](https://github.com/ocornut/imgui) (GUI)
- [STB](https://github.com/nothings/stb) (ładowanie obrazów z plików)