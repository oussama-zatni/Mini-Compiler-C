# Mini-Projet : compilateur en langage C (analyse lexicale + syntaxique)

Programme qui verifie l'analyse lexicale puis l'analyse syntaxique d'un algorithme en pseudo-code francais.

## Fichiers
- `mini_projet.c`  : code source commente
- `programme.txt`  : algorithme a analyser
- `dictionary.txt` : mots valides (mots-cles, types, symboles, lettres, chiffres)
- `errors.txt`     : genere a l'execution (erreurs lexicales / syntaxiques)
- `tests/`         : exemples d'algorithmes corrects (`ok_*`) et fautifs (`err_*`)

## Compilation et execution
    gcc mini_projet.c -o mini_projet
    ./mini_projet                         # analyse programme.txt
    ./mini_projet tests/err_lexical.txt   # analyse un autre fichier

## Correspondance avec le sujet
| Etape | Exigence | Dans le code |
|---|---|---|
| 1 | lecture caractere par caractere avec `fgetc()`, affichage sur la sortie standard | `build()` |
| 1 | compteurs du numero de ligne et du numero de caractere | `ligne`, `car` dans `build()` |
| 1 | fin de mot delimitee par un espace, caracteres ranges dans `Tab_Ligne` de 10 caracteres | `Lecteur.Tab_Ligne`, `TAILLE_TAB` |
| 2 | `Analiser_lix()` : signale la ligne et le mot fautif | `Analiser_lix()` |
| 3 | structure `Debut / Declaration / Variable / Si...alors / Sinon / Finsi / Fin` | `Analyse_syntaxique()` |
| 4 | mots dans une liste chainee, instructions terminees par `;` | `list`, `Analyse_syntaxique()` |
| 4 | `Variable Identificateur : Type ;` sur une ligne, identificateur d'un seul caractere | `analyse_declaration()` |
| 4 | `Lire` / `Ecrire` ligne par ligne, une variable par instruction | `analyse_lire()`, `analyse_ecrire()` |
| 4 | erreur signalee avec la ligne et la syntaxe | `signaler()` -> ecran + `errors.txt` |

## Format de `errors.txt`
    type error: Syntax
    line: 9
    message: l'instruction 'Ecrire' doit se terminer par ';'.

    type error: Lexical
    line: 3
    char: 14
    word: Entierr
    message: mot inconnu : il n'existe pas dans le dictionnaire.

## Auteur
Oussama ZATNI - Genie informatique - ENSA Berrechid
