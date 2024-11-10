#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <algorithm>
#include <string>
#include <random>
using namespace std;
//Population size (N), mutation rate (m), crossover rate (c)
// Typical values might be: N = 50, m = 0.05, c = 0.9
#define POP_SIZE 250
#define K_BEST_NUMBER 50
#define MAX_ITERATIONS 50
#define MUTATION_RATE 0.05
#define CROSSOVER_RATE 0.9
int MAXTIMELIMIT;


int fitnessFunction(string chromosome, vector<int> data){
    int core1Time = 0;
    int core2Time = 0;
    for (int i = 0; i < chromosome.length(); ++i) {
        if(chromosome[i] == '0'){
            core1Time +=  data[i];
        }else{
            core2Time +=  data[i];
        }
    }

    // handling infeasible solutions
    if (core1Time > MAXTIMELIMIT || core2Time > MAXTIMELIMIT) {
        return -1;
    }

    return max(core1Time, core2Time);
}


vector<string> generatePopulation(int size, int chromosomeSize, vector<int> tasks) {
    vector<string> result;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 1);

    while (size) {
        string s;
        for (int i = 0; i < chromosomeSize; ++i) {
            s += (dist(gen) == 1) ? '1' : '0';
        }
        if (fitnessFunction(s, tasks) != -1) {
            result.push_back(s);
            size--;
        }
    }
    return result;
}



void extractBest(/* in */ const vector<string>& pop, /*in*/ const vector<int>& tasks, /* out */ vector<string>& best, /* out */ vector<string>& remaining) {
    multiset<pair<int, string>> sortedPop;

    for(const string& s : pop){
        sortedPop.emplace(fitnessFunction(s, tasks), s);
    }
    int count = 0;
    for(const auto & i : sortedPop){
        if(count < K_BEST_NUMBER){
            best.push_back(i.second);
        }else{
            remaining.push_back(i.second);
        }
        count++;
    }
}

void selection(/* in */ vector<string>& population, /* in */ const vector<int>& tasks, /* out */ vector<string>& offspring){
    int totalFitness = 0;
    vector<double> rouletteWheel;

    for (const auto & i : population) {
        totalFitness += fitnessFunction(i, tasks);
    }

    rouletteWheel.reserve(population.size());
    for (const auto & i : population) {
        rouletteWheel.push_back((double)fitnessFunction(i, tasks) / totalFitness);
    }


    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dist(0.0, 1.0);
    for (int i = 0; i < K_BEST_NUMBER; ++i) {
        double selectedPercentOnTheWheel = dist(gen);
        double currentPercent = 0.0;

        for (int j = 0; j < rouletteWheel.size(); ++j) {
            currentPercent += rouletteWheel[j];

            if(selectedPercentOnTheWheel <= currentPercent){
                offspring.push_back(population[j]);
                break;
            }
        }
    }
}

void crossover(/* in and out */ vector<string>& offspring) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> crossoverDist(0.0, 1.0);
    uniform_int_distribution<> crossoverPointDist(1, offspring[0].length()-1);
    for (int i = 0; i < offspring.size(); ++i) {
        for (int j = i+1; j < offspring.size(); ++j) {
            double doingCrossoverProbability = crossoverDist(gen);
            if(doingCrossoverProbability <= CROSSOVER_RATE) {
                int positionOfCrossover = crossoverPointDist(gen);
                string firstChromosomeFirstPart = offspring[i].substr(0, positionOfCrossover);
                string firstChromosomeSecondPart = offspring[i].substr(positionOfCrossover, offspring[i].length()-1);
                string secondChromosomeFirstPart = offspring[j].substr(0, positionOfCrossover);
                string secondChromosomeSecondPart = offspring[j].substr(positionOfCrossover, offspring[j].length()-1);

                offspring[i] = firstChromosomeFirstPart + secondChromosomeSecondPart;
                offspring[j] = secondChromosomeFirstPart + firstChromosomeSecondPart;
            }
        }
    }
}

void mutation(/* in and out */ vector<string>& offspring) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> mutationDist(0.0, 1.0);
    for (int i = 0; i < offspring.size(); ++i) {
        for (int j = 0; j < offspring[i].length(); ++j) {
            double doingMutationProbability = mutationDist(gen);
            if(doingMutationProbability <= MUTATION_RATE) {
                offspring[i][j] = (offspring[i][j] == '0') ? '1' : '0';
            }
        }
    }
}

void margeBestAndOffspring(/* in */ const vector<int>& tasks, /* in and out */ vector<string>& best, /* in and out */ vector<string>& offspring) {
    for (int i = 0; i < offspring.size(); ++i) {
        for (int j = 0; j < best.size(); ++j) {
            if(fitnessFunction(offspring[i], tasks) < fitnessFunction(best[j], tasks)) {
                string temp = offspring[i];
                offspring[i] = best[i];
                best[i] = temp;
            }
        }
    }
}

void SSGA(/* in */ const vector<string>& offspring, /* in */ const vector<string>& remaining, /* out */ vector<string>& SSGAResult) {
    int offspringCount = 0.7 * remaining.size(); // 0.7 * 200 = 140
    int remainingCount = remaining.size() - offspringCount; // 200 - 140 = 60

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> offspringDist(0, 49);
    uniform_int_distribution<> remainingDist(0, 199);

    for (int i = 0; i < offspringCount; ++i) {
        int offspringIndex = offspringDist(gen);
        SSGAResult.push_back(offspring[offspringIndex]);
    }

    for (int i = 0; i < remainingCount; ++i) {
        int remainingIndex = remainingDist(gen);
        SSGAResult.push_back(remaining[remainingIndex]);
    }
}

int main() {
    fstream file;
    file.open("input.txt", ios::in);

    string s;
    getline(file, s);
    int testCasesNumber = stoi(s);

    for (int i = 0; i < testCasesNumber; ++i) {
        getline(file, s);
        MAXTIMELIMIT = stoi(s);

        getline(file, s);
        int numberOfTasks = stoi(s);

        vector<int> tasks;
        for (int j = 0; j < numberOfTasks; ++j) {
            getline(file, s);
            tasks.push_back(stoi(s));
        }

        vector<string> population = generatePopulation(POP_SIZE, numberOfTasks, tasks);
        int iteration = 0;

        while(iteration < MAX_ITERATIONS){
            vector<string> best;
            vector<string> remaining;
            vector<string> offspring;
            vector<string> SSGAResult;

            extractBest(population, tasks, best, remaining);
            selection(population, tasks, offspring);
            crossover(offspring);
            mutation(offspring);


            // handling infeasible solutions
            for (int j = 0; j < offspring.size(); ++j) {
                if (fitnessFunction(offspring[j], tasks) == -1) {
                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<> remainingDist(0, 199);
                    offspring[j] = remaining[remainingDist(gen)];
                }
            }




            margeBestAndOffspring(tasks, best, offspring);
            SSGA(offspring, remaining, SSGAResult);

            population.clear();
            for (const string& x : SSGAResult) {
                population.push_back(x);
            }

            for (const string& x : best) {
                population.push_back(x);
            }

            iteration++;
        }

        string theBest = population[0];

        for (int j = 1; j < population.size(); ++j) {
            if(fitnessFunction(theBest, tasks) > fitnessFunction(population[j], tasks)){
                theBest = population[j];
            }
        }

        vector<int> core1Tasks;
        int core1TotalTime = 0;
        vector<int> core2Tasks;
        int core2TotalTime = 0;


        for (int j = 0; j < theBest.length(); ++j) {
            if(theBest[j] == '0') {
                core1Tasks.push_back(j+1);
                core1TotalTime += tasks[j];
            }else{
                core2Tasks.push_back(j+1);
                core2TotalTime += tasks[j];
            }
        }
        cout << "-----------------------------\n";
        cout << "result for test case number #" << i+1 << '\n';
        cout << "the best chromosome is: " << theBest << '\n';
        cout << "and it's fitness value is: " << fitnessFunction(theBest,tasks) << '\n';
        cout << "the tasks assigned to core1: ";
        if(!core1Tasks.empty()){
            for (int j = 0; j < core1Tasks.size()-1; ++j) {
                cout << core1Tasks[j] << ", ";
            }
            cout << core1Tasks[core1Tasks.size()-1] << '\n';
            cout << "their total time: " << core1TotalTime << '\n';
        } else {
            cout << "None\n";
        }

        cout << "the tasks assigned to core2: ";
        if(!core2Tasks.empty()){
            for (int j = 0; j < core2Tasks.size()-1; ++j) {
                cout << core2Tasks[j] << ", ";
            }
            cout << core2Tasks[core2Tasks.size()-1] << '\n';
            cout << "their total time: " << core2TotalTime << '\n';
        } else {
            cout << "None\n";
        }
        cout << "-----------------------------\n\n\n\n";
    }

    file.close();
    return 0;
}
