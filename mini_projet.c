/* =====================================================================
 *  MINI-PROJET : Compilateur en langage C
 *  Auteur  : Oussama ZATNI
 *  Filiere : Genie informatique - ENSA Berrechid
 *  Verification de l'analyse LEXICALE puis de l'analyse SYNTAXIQUE
 *  d'un algorithme ecrit en pseudo-code francais.
 *
 *  Structure de l'algorithme accepte (Etape 3) :
 *
 *      Debut
 *      Declaration
 *      Variable v : Type ;          (une variable par ligne)
 *      Si(condition)alors
 *          Lire(v) ; / Ecrire("un message") ;
 *      Sinon
 *          Lire(v) ; / Ecrire(v) ;
 *      Finsi
 *      Fin
 *
 *  Organisation du programme :
 *    Etape 1 -> build()                : lecture caractere par caractere
 *                                        (fgetc), compteurs ligne/caractere,
 *                                        tableau Tab_Ligne de 10 caracteres
 *    Etape 2 -> Analiser_lix()         : analyse lexicale d'un mot
 *    Etape 4 -> Analyse_syntaxique()   : analyse syntaxique sur la liste
 *                                        chainee des mots
 *
 *  Compilation :  gcc mini_projet.c -o mini_projet
 *  Utilisation :  ./mini_projet [programme.txt] [dictionary.txt]
 *                 (le nom "-" pour le programme = lecture sur stdin)
 * ===================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define TAILLE_TAB       10          /* taille du tableau Tab_Ligne        */
#define TAILLE_MOT       100         /* taille maximale d'un mot           */
#define MAX_DICO         256         /* nombre maximal de mots du dico     */
#define FICHIER_ERREURS  "errors.txt"

/* ---------------------------------------------------------------------
 *  Liste chainee : chaque maillon contient un mot (token) du programme
 * --------------------------------------------------------------------- */
typedef struct list {
    char word[TAILLE_MOT];   /* le mot lu                                  */
    int size;                /* la taille du mot                           */
    int line;                /* numero de la ligne ou se trouve le mot     */
    int car;                 /* numero du caractere (dans la ligne)        */
    int est_chaine;          /* 1 si le mot est a l'interieur de "..."     */
    struct list *next;       /* maillon suivant                            */
} list;

/* Creation d'un nouveau maillon */
list *create_list(const char word[], int size, int line, int car, int est_chaine)
{
    list *newnode = (list *) malloc(sizeof(list));
    if (newnode == NULL) {
        printf("Erreur : memoire insuffisante.\n");
        exit(1);
    }
    strncpy(newnode->word, word, TAILLE_MOT - 1);
    newnode->word[TAILLE_MOT - 1] = '\0';
    newnode->size = size;
    newnode->line = line;
    newnode->car = car;
    newnode->est_chaine = est_chaine;
    newnode->next = NULL;
    return newnode;
}

/* Ajout d'un element a la fin de la liste */
list *append_at_end(list *head, const char word[], int size, int line, int car, int est_chaine)
{
    list *newnode = create_list(word, size, line, car, est_chaine);
    list *currentnode;
    if (head == NULL)
        return newnode;
    currentnode = head;
    while (currentnode->next != NULL)
        currentnode = currentnode->next;
    currentnode->next = newnode;
    return head;
}

/* Affichage du contenu de la liste */
void print_the_list(list *head)
{
    list *currentnode = head;
    if (currentnode == NULL) {
        printf("La liste est vide.\n");
        return;
    }
    while (currentnode != NULL) {
        printf("Mot: %-14s | Taille: %2d | Ligne: %2d | Caractere: %2d%s\n",
               currentnode->word, currentnode->size, currentnode->line,
               currentnode->car, currentnode->est_chaine ? " | (texte)" : "");
        currentnode = currentnode->next;
    }
}

/* Suppression du premier element de la liste */
list *remove_First_Element(list *head)
{
    list *temp;
    if (head == NULL)
        return NULL;
    temp = head;
    head = head->next;
    free(temp);
    return head;
}

/* Destruction complete de la liste */
list *List_destruction(list *head)
{
    while (head != NULL)
        head = remove_First_Element(head);
    return NULL;
}

/* ---------------------------------------------------------------------
 *  Outils
 * --------------------------------------------------------------------- */

/* Comparaison de deux chaines sans tenir compte de la casse
   (Debut = debut, Lire = lire ...). Retourne 1 si egales. */
static int ci_egal(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char) *a) != tolower((unsigned char) *b))
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

/* Un identificateur doit avoir UN SEUL caractere : une lettre */
int isidentifier(const char *word)
{
    if (strlen(word) != 1)
        return 0;
    return isalpha((unsigned char) word[0]) ? 1 : 0;
}

/* Un mot est un nombre : chiffres avec au plus un point decimal */
static int est_nombre(const char *w)
{
    int i, chiffres = 0, points = 0;
    for (i = 0; w[i] != '\0'; i++) {
        if (isdigit((unsigned char) w[i]))
            chiffres++;
        else if (w[i] == '.')
            points++;
        else
            return 0;
    }
    return chiffres > 0 && points <= 1;
}

/* Caracteres qui terminent un mot */
static int est_separateur(int c)
{
    return c == ' '  || c == '\n' || c == '\t' || c == '\r' ||
           c == '('  || c == ')'  || c == ';'  || c == ':'  ||
           c == ','  || c == '"'  || c == '<'  || c == '>'  ||
           c == '='  || c == '!';
}

/* ---------------------------------------------------------------------
 *  Signalement des erreurs : ecran + fichier errors.txt
 * --------------------------------------------------------------------- */
static void signaler(const char *type, int ligne, int car, const char *mot, const char *message)
{
    FILE *f;
    printf("[Erreur %s] ligne %d", type, ligne);
    if (car > 0)
        printf(", caractere %d", car);
    if (mot != NULL)
        printf(", mot '%s'", mot);
    printf(" : %s\n", message);

    f = fopen(FICHIER_ERREURS, "a");
    if (f != NULL) {
        fprintf(f, "type error: %s\nline: %d\n", type, ligne);
        if (car > 0)
            fprintf(f, "char: %d\n", car);
        if (mot != NULL)
            fprintf(f, "word: %s\n", mot);
        fprintf(f, "message: %s\n\n", message);
        fclose(f);
    }
}

/* =====================================================================
 *  ETAPE 1 : LECTURE DU PROGRAMME
 *  - lecture caractere par caractere avec fgetc()
 *  - le contenu lu est affiche sur la sortie standard
 *  - compteurs : numero de la ligne et numero du caractere
 *  - la fin d'un mot est delimitee par un espace (ou un symbole)
 *  - les caracteres lus sont mis dans le tableau Tab_Ligne (10 car.)
 * ===================================================================== */
typedef struct {
    char Tab_Ligne[TAILLE_TAB + 1];  /* tableau de 10 caracteres (+ '\0')   */
    int  nb;                         /* nombre de caracteres dans Tab_Ligne */
    char mot[TAILLE_MOT];            /* mot en cours de construction        */
    int  lg;                         /* longueur du mot en cours            */
    int  car_mot;                    /* numero du 1er caractere du mot      */
} Lecteur;

/* Vide Tab_Ligne dans le mot en cours (quand le tableau est plein
   ou quand le mot se termine) */
static void transferer_tab(Lecteur *L)
{
    int i;
    for (i = 0; i < L->nb; i++) {
        if (L->lg < TAILLE_MOT - 1)
            L->mot[L->lg++] = L->Tab_Ligne[i];
    }
    L->mot[L->lg] = '\0';
    L->nb = 0;
    L->Tab_Ligne[0] = '\0';
}

/* Termine le mot en cours : il est ajoute a la liste chainee */
static list *terminer_mot(Lecteur *L, list *head, int ligne, int est_chaine)
{
    if (L->nb > 0)
        transferer_tab(L);
    if (L->lg > 0) {
        head = append_at_end(head, L->mot, L->lg, ligne, L->car_mot, est_chaine);
        L->lg = 0;
        L->mot[0] = '\0';
    }
    return head;
}

/* Construction de la liste des mots a partir du fichier programme */
list *build(list *head, FILE *p)
{
    Lecteur L;
    int ligne = 1;        /* compteur : numero de la ligne                 */
    int car = 0;          /* compteur : numero du caractere dans la ligne  */
    int en_chaine = 0;    /* 1 si on est entre deux guillemets             */
    int c, next;
    char symbole[3];

    L.nb = 0; L.lg = 0; L.car_mot = 0;
    L.Tab_Ligne[0] = '\0';
    L.mot[0] = '\0';

    printf("------------- Lecture du programme -------------\n");
    while ((c = fgetc(p)) != EOF) {
        putchar(c);                       /* affichage sur la sortie standard */
        car++;                            /* compteur de caracteres           */

        if (!est_separateur(c)) {
            /* caractere ordinaire : on le range dans Tab_Ligne */
            if (L.nb == 0 && L.lg == 0)
                L.car_mot = car;
            L.Tab_Ligne[L.nb++] = (char) c;
            L.Tab_Ligne[L.nb] = '\0';
            if (L.nb == TAILLE_TAB)       /* tableau plein : on le vide       */
                transferer_tab(&L);
            continue;
        }

        /* un separateur termine le mot en cours */
        head = terminer_mot(&L, head, ligne, en_chaine);

        if (c == '\n') {                  /* fin de ligne                     */
            ligne++;
            car = 0;
            en_chaine = 0;                /* un texte ne peut pas continuer   */
        }
        else if (c == '<' || c == '>' || c == '=' || c == '!') {
            /* operateurs : <, >, =, !  ou  <=, >=, ==, != */
            symbole[0] = (char) c;
            symbole[1] = '\0';
            next = fgetc(p);
            if (next == '=') {
                putchar(next);
                car++;
                symbole[1] = '=';
                symbole[2] = '\0';
            } else if (next != EOF) {
                ungetc(next, p);
            }
            head = append_at_end(head, symbole, (int) strlen(symbole),
                                 ligne, car - (int) strlen(symbole) + 1, en_chaine);
        }
        else if (c == '"') {
            /* les guillemets eux-memes sont des symboles de syntaxe */
            head = append_at_end(head, "\"", 1, ligne, car, 0);
            en_chaine = !en_chaine;
        }
        else if (c == '(' || c == ')' || c == ';' || c == ':' || c == ',') {
            symbole[0] = (char) c;
            symbole[1] = '\0';
            head = append_at_end(head, symbole, 1, ligne, car, en_chaine);
        }
        /* espace, tabulation, retour chariot : rien d'autre a faire */
    }
    /* dernier mot du fichier (s'il n'y a pas de separateur final) */
    head = terminer_mot(&L, head, ligne, en_chaine);
    printf("\n------------------------------------------------\n");
    printf("Lecture terminee : %d ligne(s).\n\n", ligne);
    return head;
}

/* =====================================================================
 *  ETAPE 2 : ANALYSE LEXICALE
 * ===================================================================== */
static char dico[MAX_DICO][TAILLE_MOT];   /* le dictionnaire en memoire */
static int  nb_dico = 0;

/* Chargement du dictionnaire (une seule lecture du fichier) */
int charger_dictionnaire(const char *chemin)
{
    FILE *f = fopen(chemin, "r");
    if (f == NULL)
        return 0;
    while (nb_dico < MAX_DICO && fscanf(f, "%99s", dico[nb_dico]) == 1)
        nb_dico++;
    fclose(f);
    return 1;
}

/* Analiser_lix : analyse lexicale du mot contenu dans le tableau d'entree.
   Le mot est valide s'il existe dans le dictionnaire ou si c'est un nombre.
   Sinon l'erreur est signalee avec la LIGNE et le MOT fautif.
   Retourne 1 si le mot est correct, 0 sinon. */
int Analiser_lix(const char Tab_Ligne[], int ligne, int car)
{
    int i;
    for (i = 0; i < nb_dico; i++) {
        if (ci_egal(Tab_Ligne, dico[i]))
            return 1;
    }
    if (est_nombre(Tab_Ligne))
        return 1;
    signaler("Lexical", ligne, car, Tab_Ligne,
             "mot inconnu : il n'existe pas dans le dictionnaire.");
    return 0;
}

/* Analyse lexicale de toute la liste (les textes entre guillemets
   ne sont pas analyses). Retourne le nombre d'erreurs. */
int analyse_lexicale_liste(list *head)
{
    int erreurs = 0;
    list *cur;
    for (cur = head; cur != NULL; cur = cur->next) {
        if (cur->est_chaine)
            continue;
        if (!Analiser_lix(cur->word, cur->line, cur->car))
            erreurs++;
    }
    return erreurs;
}

/* =====================================================================
 *  ETAPE 4 : ANALYSE SYNTAXIQUE (descente recursive sur la liste chainee)
 *
 *  programme    := Debut Declaration declaration* instruction* Fin
 *  declaration  := Variable id : Type ;            (sur UNE seule ligne)
 *  instruction  := Lire ( id ) ;                   (sur sa propre ligne)
 *                | Ecrire ( "texte" | id ) ;       (sur sa propre ligne)
 *                | Si ( operande op operande ) alors instruction*
 *                  [ Sinon instruction* ] Finsi
 *  id           := une seule lettre, deja declaree
 * ===================================================================== */
static list *cur  = NULL;        /* mot courant                           */
static list *prec = NULL;        /* dernier mot consomme                  */
static int   derniere_ligne = 1; /* ligne du dernier mot du programme     */
static char  declaree[256];      /* declaree[c] = 1 si la variable c existe */

static void avancer(void)
{
    if (cur != NULL) {
        prec = cur;
        cur = cur->next;
    }
}

/* le mot courant est-il le mot cle (ou symbole) attendu ? */
static int est(const list *t, const char *mot)
{
    return t != NULL && !t->est_chaine && ci_egal(t->word, mot);
}

/* Erreur sur le mot COURANT (mot inattendu) */
static int err_ici(const char *fmt, ...)
{
    char msg[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    signaler("Syntax", cur ? cur->line : derniere_ligne, cur ? cur->car : 0, NULL, msg);
    return 0;
}

/* Erreur apres le dernier mot lu (quelque chose manque a cet endroit) */
static int err_apres(const char *fmt, ...)
{
    char msg[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    signaler("Syntax", prec ? prec->line : derniere_ligne, 0, NULL, msg);
    return 0;
}

/* Consomme le mot attendu, sinon signale l'erreur "msg" */
static int attendre(const char *mot, const char *msg)
{
    if (est(cur, mot)) {
        avancer();
        return 1;
    }
    return err_apres("%s", msg);
}

/* Lit un identificateur (une seule lettre). Le caractere est dans
   prec->word[0] apres un appel reussi. */
static int lire_identificateur(int doit_etre_declaree)
{
    if (cur == NULL)
        return err_apres("identificateur attendu.");
    if (cur->est_chaine || !isidentifier(cur->word))
        return err_ici("identificateur invalide '%s' : il doit avoir un seul caractere (lettre).",
                       cur->word);
    if (doit_etre_declaree && !declaree[(unsigned char) cur->word[0]])
        return err_ici("la variable '%s' n'est pas declaree.", cur->word);
    avancer();
    return 1;
}

/* declaration := Variable id : Type ;   (tout sur la meme ligne) */
static int analyse_declaration(void)
{
    int ligne = cur->line;
    char id;
    avancer();                                         /* Variable */
    if (!lire_identificateur(0))
        return 0;
    id = prec->word[0];
    if (declaree[(unsigned char) id])
        return err_apres("la variable '%c' est deja declaree.", id);
    if (!attendre(":", "':' attendu apres l'identificateur (Variable v : Type ;)."))
        return 0;
    if (!(est(cur, "Entier") || est(cur, "Reel"))) {
        if (cur == NULL)
            return err_apres("type attendu ('Entier' ou 'Reel').");
        return err_ici("type invalide '%s' : 'Entier' ou 'Reel' attendu.", cur->word);
    }
    avancer();
    if (!attendre(";", "la declaration d'une variable doit se terminer par ';'."))
        return 0;
    if (prec->line != ligne) {
        signaler("Syntax", ligne, 0, NULL,
                 "la declaration d'une variable doit tenir sur une seule ligne.");
        return 0;
    }
    declaree[(unsigned char) id] = 1;
    return 1;
}

/* operande d'une condition : variable declaree ou nombre */
static int analyse_operande(void)
{
    if (cur == NULL)
        return err_apres("operande attendu dans la condition.");
    if (!cur->est_chaine && isidentifier(cur->word))
        return lire_identificateur(1);
    if (!cur->est_chaine && est_nombre(cur->word)) {
        avancer();
        return 1;
    }
    return err_ici("operande invalide '%s' dans la condition.", cur->word);
}

/* Lire ( id ) ;   -> une seule variable, sur sa propre ligne */
static int analyse_lire(void)
{
    if (prec != NULL && prec->line == cur->line)
        return err_ici("'Lire' doit etre sur sa propre ligne (une instruction par ligne).");
    avancer();
    if (!attendre("(", "'(' attendue apres 'Lire'."))
        return 0;
    if (!lire_identificateur(1))
        return 0;
    if (cur != NULL && est(cur, ","))
        return err_ici("'Lire' accepte une seule variable par instruction.");
    if (!attendre(")", "')' manquante dans 'Lire'."))
        return 0;
    return attendre(";", "l'instruction 'Lire' doit se terminer par ';'.");
}

/* Ecrire ( "message" | id ) ;   -> sur sa propre ligne */
static int analyse_ecrire(void)
{
    if (prec != NULL && prec->line == cur->line)
        return err_ici("'Ecrire' doit etre sur sa propre ligne (une instruction par ligne).");
    avancer();
    if (!attendre("(", "'(' attendue apres 'Ecrire'."))
        return 0;
    if (est(cur, "\"")) {
        int ligne = cur->line;
        avancer();                                     /* guillemet ouvrant */
        while (cur != NULL && cur->est_chaine)
            avancer();                                 /* le message        */
        if (!est(cur, "\"") || cur->line != ligne) {
            signaler("Syntax", ligne, 0, NULL, "guillemet fermant manquant dans 'Ecrire'.");
            return 0;
        }
        avancer();                                     /* guillemet fermant */
    } else if (!lire_identificateur(1)) {
        return 0;
    }
    if (cur != NULL && est(cur, ","))
        return err_ici("'Ecrire' accepte une seule variable par instruction.");
    if (!attendre(")", "')' manquante dans 'Ecrire'."))
        return 0;
    return attendre(";", "l'instruction 'Ecrire' doit se terminer par ';'.");
}

static int analyse_liste_instructions(void);

/* Si ( cond ) alors ... [ Sinon ... ] Finsi */
static int analyse_si(void)
{
    int ligne = cur->line;
    char msg[128];
    avancer();                                         /* Si */
    if (!attendre("(", "'(' attendue apres 'Si' : Si(condition)alors."))
        return 0;
    if (!analyse_operande())
        return 0;
    if (!(est(cur, "<") || est(cur, "<=") || est(cur, ">") || est(cur, ">=") ||
          est(cur, "==") || est(cur, "!=") || est(cur, "="))) {
        if (cur == NULL)
            return err_apres("operateur de comparaison attendu.");
        return err_ici("operateur de comparaison attendu (<, <=, >, >=, ==, !=) au lieu de '%s'.",
                       cur->word);
    }
    avancer();
    if (!analyse_operande())
        return 0;
    if (!attendre(")", "')' attendue a la fin de la condition."))
        return 0;
    if (!attendre("alors", "'alors' attendu apres la condition."))
        return 0;
    if (!analyse_liste_instructions())
        return 0;
    if (est(cur, "Sinon")) {
        avancer();
        if (!analyse_liste_instructions())
            return 0;
    }
    snprintf(msg, sizeof msg, "'Finsi' attendu pour fermer le 'Si' de la ligne %d.", ligne);
    return attendre("Finsi", msg);
}

/* une instruction : Lire, Ecrire ou Si */
static int analyse_instruction(void)
{
    if (est(cur, "Lire"))
        return analyse_lire();
    if (est(cur, "Ecrire") || est(cur, "\xc3\xa9" "crire"))   /* Ecrire / ecrire accentue */
        return analyse_ecrire();
    if (est(cur, "Si"))
        return analyse_si();
    if (est(cur, "Variable"))
        return err_ici("les variables se declarent apres 'Declaration', avant les instructions.");
    return err_ici("instruction inconnue ou incorrecte : '%s'.", cur->word);
}

/* instruction* : s'arrete devant Sinon, Finsi, Fin ou la fin du fichier */
static int analyse_liste_instructions(void)
{
    while (cur != NULL && !est(cur, "Sinon") && !est(cur, "Finsi") && !est(cur, "Fin")) {
        if (!analyse_instruction())
            return 0;
    }
    return 1;
}

/* Analyse_syntaxique : verifie la structure complete du programme.
   Retourne 1 si le programme est correct, 0 sinon (premiere erreur). */
int Analyse_syntaxique(list *head)
{
    list *t;
    memset(declaree, 0, sizeof declaree);
    cur = head;
    prec = NULL;
    for (t = head; t != NULL; t = t->next)
        derniere_ligne = t->line;

    if (cur == NULL) {
        signaler("Syntax", 1, 0, NULL, "le programme est vide.");
        return 0;
    }
    if (!est(cur, "Debut"))
        return err_ici("le programme doit commencer par 'Debut'.");
    avancer();
    if (!attendre("Declaration", "'Declaration' attendu apres 'Debut'."))
        return 0;
    while (est(cur, "Variable")) {
        if (!analyse_declaration())
            return 0;
    }
    if (!analyse_liste_instructions())
        return 0;
    if (cur == NULL)
        return err_apres("'Fin' manquant a la fin du programme.");
    if (!est(cur, "Fin"))
        return err_ici("'%s' sans 'Si' correspondant.", cur->word);
    avancer();
    if (cur != NULL)
        return err_ici("aucune instruction ne doit suivre 'Fin'.");
    return 1;
}

/* =====================================================================
 *  PROGRAMME PRINCIPAL
 * ===================================================================== */
int main(int argc, char *argv[])
{
    const char *programme  = (argc > 1) ? argv[1] : "programme.txt";
    const char *dictionnaire = (argc > 2) ? argv[2] : "dictionary.txt";
    FILE *p, *f;
    list *head = NULL;
    int erreurs_lex;

    /* le fichier d'erreurs est remis a zero a chaque execution */
    f = fopen(FICHIER_ERREURS, "w");
    if (f != NULL)
        fclose(f);

    if (strcmp(programme, "-") == 0) {
        p = stdin;
    } else {
        p = fopen(programme, "r");
        if (p == NULL) {
            printf("Erreur : impossible d'ouvrir le fichier programme '%s'.\n", programme);
            return 1;
        }
    }
    if (!charger_dictionnaire(dictionnaire)) {
        printf("Erreur : impossible d'ouvrir le dictionnaire '%s'.\n", dictionnaire);
        return 1;
    }

    /* Etape 1 : lecture */
    head = build(head, p);
    if (p != stdin)
        fclose(p);
    print_the_list(head);

    /* Etape 2 : analyse lexicale */
    printf("\n=== Analyse lexicale ===\n");
    erreurs_lex = analyse_lexicale_liste(head);
    if (erreurs_lex == 0)
        printf("Analyse lexicale reussie.\n");
    else
        printf("Analyse lexicale echouee : %d erreur(s). Voir %s.\n", erreurs_lex, FICHIER_ERREURS);

    /* Etapes 3 et 4 : analyse syntaxique (seulement si le lexical est correct) */
    printf("\n=== Analyse syntaxique ===\n");
    if (erreurs_lex > 0) {
        printf("Analyse syntaxique non lancee : corrigez d'abord les erreurs lexicales.\n");
    } else if (Analyse_syntaxique(head)) {
        printf("Analyse syntaxique reussie.\n");
    } else {
        printf("Analyse syntaxique echouee. Voir %s.\n", FICHIER_ERREURS);
    }

    head = List_destruction(head);
    return (erreurs_lex > 0) ? 2 : 0;
}
