#include "Node.hpp"
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>

extern bool main_defined;
extern std::multiset<std::string> declared_functions;

// production functions
int primarni_izraz(std::shared_ptr<Node> root);
int postfiks_izraz(std::shared_ptr<Node> root);
int lista_argumenata(std::shared_ptr<Node> root);
int unarni_izraz(std::shared_ptr<Node> root);
int unarni_operator(std::shared_ptr<Node> root);
int cast_izraz(std::shared_ptr<Node> root);
int ime_tipa(std::shared_ptr<Node> root);
int specifikator_tipa(std::shared_ptr<Node> root);
int multiplikativni_izraz(std::shared_ptr<Node> root);
int aditivni_izraz(std::shared_ptr<Node> root);
int odnosni_izraz(std::shared_ptr<Node> root);
int jednakosni_izraz(std::shared_ptr<Node> root);
int bin_i_izraz(std::shared_ptr<Node> root);
int bin_xili_izraz(std::shared_ptr<Node> root);
int bin_ili_izraz(std::shared_ptr<Node> root);
int log_i_izraz(std::shared_ptr<Node> root);
int log_ili_izraz(std::shared_ptr<Node> root);
int izraz_pridruzivanja(std::shared_ptr<Node> root);
int izraz(std::shared_ptr<Node> root);
int slozena_naredba(std::shared_ptr<Node> root);
int lista_naredbi(std::shared_ptr<Node> root);
int naredba(std::shared_ptr<Node> root);
int izraz_naredba(std::shared_ptr<Node> root);
int naredba_grananja(std::shared_ptr<Node> root);
int naredba_petlje(std::shared_ptr<Node> root);
int naredba_skoka(std::shared_ptr<Node> root);
int prijevodna_jedinica(std::shared_ptr<Node> root);
int vanjska_deklaracija(std::shared_ptr<Node> root);
int definicija_funkcije(std::shared_ptr<Node> root);
int lista_parametara(std::shared_ptr<Node> root);
int deklaracija_parametra(std::shared_ptr<Node> root);
int lista_deklaracija(std::shared_ptr<Node> root);
int deklaracija(std::shared_ptr<Node> root);
int lista_init_deklaratora(std::shared_ptr<Node> root);
int init_deklarator(std::shared_ptr<Node> root);
int izravni_deklarator(std::shared_ptr<Node> root);
int inicijalizator(std::shared_ptr<Node> root);
int lista_izraza_pridruzivanja(std::shared_ptr<Node> root);
