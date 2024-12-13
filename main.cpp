#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip> 

using namespace std;


vector<vector<double>> loadRatings(const string& filename) {
    vector<vector<double>> ratings;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Unable to open the file." << endl;
        return ratings; 
    }

    string line;
    bool firstRow = true;

    
    while (getline(file, line)) {
        if (firstRow) {
            firstRow = false; // Skip the header row
            continue;
        }

        stringstream ss(line);
        string value;
        vector<double> row;
        bool firstColumn = true; // Skip the Users column

        while (getline(ss, value, ',')) {
            if (firstColumn) {
                firstColumn = false;
                continue;
            }
            row.push_back(stod(value));
        }
        ratings.push_back(row);
    }
    return ratings;
}


double cosineSimilarity(const vector<double>& user1, const vector<double>& user2) {
    double dotProduct = 0.0, normA = 0.0, normB = 0.0;
    for (size_t i = 0; i < user1.size(); ++i) {
        dotProduct += user1[i] * user2[i];
        normA += pow(user1[i], 2);
        normB += pow(user2[i], 2);
    }
    return (normA > 0 && normB > 0) ? dotProduct / (sqrt(normA) * sqrt(normB)) : 0.0;
}


vector<vector<double>> normalizeRatings(const vector<vector<double>>& ratings, vector<double>& userMeans) {
    vector<vector<double>> normalizedRatings = ratings;
    userMeans.resize(ratings.size(), 0.0);

    for (size_t i = 0; i < ratings.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        for (double rating : ratings[i]) {
            if (rating > 0) {
                sum += rating;
                count++;
            }
        }
        userMeans[i] = (count > 0) ? sum / count : 0.0;

        for (size_t j = 0; j < ratings[i].size(); ++j) {
            if (ratings[i][j] > 0) {
                normalizedRatings[i][j] -= userMeans[i];
            }
        }
    }
    return normalizedRatings;
}


vector<double> predictRatings(const vector<vector<double>>& ratings, int userIndex, const vector<double>& userMeans) {
    vector<double> predictions(ratings[userIndex].size(), 0.0);
    vector<double> similarity(ratings.size(), 0.0);

    for (size_t i = 0; i < ratings.size(); ++i) {
        if (i == userIndex) continue;
        similarity[i] = cosineSimilarity(ratings[userIndex], ratings[i]);
    }

    for (size_t movie = 0; movie < ratings[userIndex].size(); ++movie) {
        
            
            double numerator = 0.0, denominator = 0.0;
            for (size_t otherUser = 0; otherUser < ratings.size(); ++otherUser) {
               
                    numerator += similarity[otherUser] * ratings[otherUser][movie];
                    denominator += abs(similarity[otherUser]);
                
            }
            predictions[movie] = userMeans[userIndex] + ((denominator > 0) ? numerator / denominator : 0.0);
        }
    
    return predictions;
}



vector<int> recommendTopN(const vector<double>& predictions, int N) {
    vector<pair<double, int>> movieScores;
    for (size_t i = 0; i < predictions.size(); ++i) {

        movieScores.push_back({predictions[i], i});
    }
    sort(movieScores.rbegin(), movieScores.rend());
    vector<int> topN;
    for (int i = 0; i < N && i < movieScores.size(); ++i) {
        topN.push_back(movieScores[i].second);
    }
    return topN;
}


double calculateRMSE(const vector<vector<double>>& ratings, const vector<vector<double>>& predictions) {
    double errorSum = 0.0;
    int count = 0;

    for (size_t i = 0; i < ratings.size(); ++i) {
        for (size_t j = 0; j < ratings[i].size(); ++j) {
            if (ratings[i][j] != 0) {
                double error = ratings[i][j] - predictions[i][j];
                errorSum += error * error;
                count++;
            }
        }
    }
    return (count > 0) ? sqrt(errorSum / count) : 0.0;
}


int main() {
    string filename = "ratings.csv";
    vector<vector<double>> ratings = loadRatings(filename);

    if (ratings.empty()) {
        return 1; 
    }

    vector<double> userMeans;
    vector<vector<double>> normalizedRatings = normalizeRatings(ratings, userMeans);

    int userIndex;
    cout << "Enter the user index (1-based): ";
    cin >> userIndex;
    userIndex--; 

    if (userIndex < 0 || userIndex >= ratings.size()) {
        cerr << "Invalid user index." << endl;
        return 1;
    }

    int topN;
    cout << "Enter the number of top recommendations: ";
    cin >> topN;

    vector<double> predictions = predictRatings(normalizedRatings, userIndex, userMeans);
    vector<int> topMovies = recommendTopN(predictions, topN);

    cout << "Predicted ratings for unrated movies for User " << userIndex + 1 << ":\n";
    for (size_t i = 0; i < predictions.size(); ++i) {
        if (ratings[userIndex][i] == 0) {
            cout << "Movie " << i + 1 << ": " << fixed << setprecision(2) << predictions[i] << endl;
        }
    }

    cout << "\nTop " << topN << " recommendations for User " << userIndex + 1 << ":\n";
    for (int movie : topMovies) {
        cout << "Movie " << movie + 1 << " (Predicted Rating: " << fixed << setprecision(2) << predictions[movie] << ")" << endl;
    }

    
    vector<vector<double>> allPredictions(ratings.size(), vector<double>(ratings[0].size(), 0.0));
    for (size_t i = 0; i < ratings.size(); ++i) {
        allPredictions[i] = predictRatings(normalizedRatings, i, userMeans);
    }

    double rmse = calculateRMSE(ratings, allPredictions);
    cout << "\nPerformance Report:\n";
    cout << "Root Mean Square Error (RMSE): " << fixed << setprecision(4) << rmse << endl;

    return 0;
}

