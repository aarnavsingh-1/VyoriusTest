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