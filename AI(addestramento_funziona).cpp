#include<iostream>
#include<random>
#include<unordered_map>
#include<fstream>
#include<string>
#include<ctime>
using namespace std;

// Griglia di gioco globale
int bottoni[3][3] = {
    {0,0,0},
    {0,0,0},
    {0,0,0}
};

bool turno = true; 

int vincitore() {
    int G1 = 1, G2 = 2, G3 = 3; // G3 = pareggio
    for (int i = 0; i < 3; i++) {
        if (bottoni[i][0] == bottoni[i][1] && bottoni[i][1] == bottoni[i][2] && bottoni[i][0] != 0)
            return bottoni[i][0];
    }
    for (int j = 0; j < 3; j++) {
        if (bottoni[0][j] == bottoni[1][j] && bottoni[1][j] == bottoni[2][j] && bottoni[0][j] != 0)
            return bottoni[0][j];
    }
    if (bottoni[0][0] == bottoni[1][1] && bottoni[1][1] == bottoni[2][2] && bottoni[0][0] != 0)
        return bottoni[0][0];
    if (bottoni[0][2] == bottoni[1][1] && bottoni[1][1] == bottoni[2][0] && bottoni[0][2] != 0)
        return bottoni[0][2];

    bool pieno = true;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (bottoni[i][j] == 0) pieno = false;

    if (pieno) return G3; // pareggio
    return 0;
}

void reset() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            bottoni[i][j] = 0;
    turno = true; // sempre inizia il Giocatore 1
}

class AI {
public:
    unordered_map<string, unordered_map<string, float>> q_values; // Q-table
    vector<pair<string, string>> history; // cronologia delle mosse della AI
    float alpha = 0.2f;   // Tasso di apprendimento
    float gamma = 0.99f;   // Fattore di sconto (non usato in questa versione)
    float esplorazione = 0.9f; // Probabilità di mossa casuale

    string stato(int bottoni[3][3]) {
        string stato = "";
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                stato += to_string(bottoni[i][j]);
        return stato;
    }

    float reward(int vincitore) {
        if (vincitore == 2) return 1.0;
        if (vincitore == 1) return -2.0;
        if (vincitore == 3) return 0.5;
        return 0.0;
    }

    void azione() {
        string s = stato(bottoni);

        if (q_values[s].empty()) {
            for (int i = 0; i < 9; ++i)
                q_values[s][to_string(i)] = 0.0f;
        }

        int mossa = -1;

        if ((float)rand() / RAND_MAX < esplorazione) {
            // Mossa casuale
            do {
                mossa = rand() % 9;
            } while (bottoni[mossa / 3][mossa % 3] != 0);
        } else {
            // Miglior mossa secondo Q-table
            float max_q = -999;
            bool mossa_trovata = false;
            for (auto& [azione, valore] : q_values[s]) {
                int idx = stoi(azione);
                int r = idx / 3, c = idx % 3;
                if (bottoni[r][c] == 0 && valore > max_q) {
                    max_q = valore;
                    mossa = idx;
                    mossa_trovata = true;
                }
            }
            if (!mossa_trovata) {
                do {
                    mossa = rand() % 9;
                } while (bottoni[mossa / 3][mossa % 3] != 0);
            }
        }

        // Applica la mossa
        bottoni[mossa / 3][mossa % 3] = 2;

        // Salva la mossa per aggiornamento successivo
        history.push_back({s, to_string(mossa)});
    }

    void aggiorna_qtable(int vincitore) {
        float r = reward(vincitore);
        for (int i = history.size() - 1; i >= 0; --i) {
            string s = history[i].first;
            string a = history[i].second;
            float q_attuale = q_values[s][a];
            q_values[s][a] = q_attuale + alpha * (r - q_attuale);
            r *= gamma; 
        }
        history.clear();
    }

    void salva_qtable(const string& nomefile) {
        ofstream file(nomefile);
        for (auto& [s, azioni] : q_values)
            for (auto& [a, valore] : azioni)
                file << s << " " << a << " " << valore << endl;
        file.close();
    }
};

int main() {
    srand(static_cast<unsigned int>(time(0))); 

    AI ai;
    int partite = 1000000;
    for (int i = 0; i < partite; i++) {
        while (vincitore() == 0) {
            if (turno) {
                int mossa;
                do {
                    mossa = rand() % 9;
                } while (bottoni[mossa / 3][mossa % 3] != 0);
                bottoni[mossa / 3][mossa % 3] = 1;
                turno = false;
            } else {
                ai.azione();
                turno = true;
            }
        }
        int risultato = vincitore();
        ai.aggiorna_qtable(risultato);
        reset();
        ai.alpha *= 0.99999; // Decrescita del tasso di apprendimento
        ai.esplorazione *= 0.99999; // Decrescita della probabilità di esplorazione
        cout << "Partita " << i  << endl;
    }

    ai.salva_qtable("q_values.txt");

    cout << "Allenamento completato: " << partite << " partite giocate." << endl;

    return 0;
}
