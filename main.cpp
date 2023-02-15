#include <bits/stdc++.h>

using namespace std;

/// Structuri
struct files {
    string name;
    int size;
};
struct rets {
    vector<vector<set<int>>> ret;
};
struct mapper{
    int R;
    int thread_id;
    int *index;
    pthread_mutex_t *mutex;
    pthread_barrier_t *barrier;
    vector<string> numeFisiere;
    rets *ret;
};
struct reducer{
    int M;
    int thread_id;
    pthread_barrier_t *barrier;
    rets *ret;
};

/// Algoritmul lui Newton
double quickPower(double base, int exponent)
{
    if(!exponent)
        return 1.0;

    if((exponent % 2))
        return base*quickPower(base*base, exponent/2);
    else
        return quickPower(base*base, exponent/2);
}
unsigned long long int quickPow(unsigned long int base, unsigned long long int exponent)
{
    if(!exponent)
        return 1LL;

    if((exponent % 2))
        return base*quickPow(base*base, exponent/2);
    else
        return quickPow(base*base, exponent/2);
}
double root(double number, double order, double error = 0.1) {
    if (order == 2.0)
        return sqrt(number);

    double x = 1;
    while (true) {
        double h;
        h = (quickPower(x, (int) order) - number);
        h /= (order * quickPower(x, (int) order - 1));
        if (abs(h) < error)
            return x;
        x = x - h;
    }
}
bool isPerfectPower(unsigned long long int number, unsigned long long int exponent)
{
    if( number == 1 )
        return true;

    auto val = (unsigned long long int) root(number, exponent);
    bool ok = false;
    for( unsigned long long int i = val - 2; i <= val + 2; i++ )
        ok = ok || (number == quickPow(i, exponent));
    return ok;
}

/// Mapper & Reducer
void* Mapper(void* arg) {
    auto aux = (mapper*) arg;
    while(*aux->index < (int) aux->numeFisiere.size()) {
        // Citirea Dinamica
        pthread_mutex_lock(aux->mutex);
        ifstream extr(aux->numeFisiere[(*aux->index)++]);
        pthread_mutex_unlock(aux->mutex);
        // Proces Mapper
        int n; extr >> n;
        for (int i = 0; i < n; i++) {
            int x; extr >> x;
            for (int j = 0; j < aux->R; j++) {
                if (isPerfectPower(x, j + 2)) {
                    aux->ret->ret[aux->thread_id][j].insert(x);
                }
            }
        }
        extr.close();
    }
    pthread_barrier_wait(aux->barrier);
    return nullptr;
}
void* Reducer(void* arg) {
    auto aux = (reducer*) arg;
    set<int> rett;
    pthread_barrier_wait(aux->barrier);
    // Proces Reducer
    for (int i = 0; i < aux->M; i++) {
        for(auto x : aux->ret->ret[i][aux->thread_id]) {
            rett.insert(x);
        }
    }
    // Creaza fisiere de iesire
    string out = "out" + to_string(aux->thread_id + 2) + ".txt";
    ofstream jake(out);
    jake << rett.size();
    jake.close();
    return nullptr;
}

/// Main
int main(int argv, char** argc) {
    /// Definitii
    int M, R; string numeFisier(argc[3]);
    M = stoi(argc[1]);
    R = stoi(argc[2]);
    pthread_t threadM[M], threadR[R];
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, nullptr, M+R);
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);
    vector<string> fisiere; string x;
    /// Citire
    ifstream fin(numeFisier);
    int n; fin >> n;
    vector<files> citire;
    for (int i = 0; i < n; i++) {
        fin >> x;
        ifstream getN (x);
        int N;
        getN >> N;
        files temp;
        temp.name = x; temp.size = N; bool okky = false;
        // Sortare descrescator
        if (citire.empty())
            citire.push_back(temp);
        else
            for (int j = 0; j < (int) citire.size(); j++) {
                if (citire[j].size <= N) {
                    citire.insert(citire.begin() + j, temp);
                    okky = true;
                    break;
                }
            }
        if (!okky)
            citire.push_back(temp);
    }
    // Introducerea valorilor sortate in
     // Vectorul de fisiere care va fi folosit de functii
    for (auto & i : citire) {
        x = i.name;
        fisiere.push_back(x);
    }

    /// pthread_Creates
    rets ret;
    ret.ret = vector<vector<set<int>>>( M,  vector<set<int>>(R) );
    int index = 0;
    for (int i = 0; i < M + R; i++) {
        if (i < M) {
            auto aux = new mapper;
            aux->R = R;
            aux->thread_id = i;
            aux->index = &index;
            aux->mutex = &mutex;
            aux->barrier = &barrier;
            aux->numeFisiere = fisiere;
            aux->ret = &ret;
            pthread_create(&threadM[i], nullptr, Mapper, aux);
        }
        else
        {
            i = i - M;
            auto auxR = new reducer;
            auxR->M = M;
            auxR->thread_id = i;
            auxR->barrier = &barrier;
            auxR->ret = &ret;
            pthread_create(&threadR[i], nullptr, Reducer, auxR);
            i = i + M;
        }
    }
    /// pthread_Joins
    for (int i = 0; i < M + R; i++) {
        if (i < M)
            pthread_join(threadM[i], nullptr);
        else {
            i = i - M;
            pthread_join(threadR[i], nullptr);
            i = i + M;
        }
    }

    pthread_mutex_destroy(&mutex);
    pthread_barrier_destroy(&barrier);
    fin.close();
    return 0;
}