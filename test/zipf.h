#include    "../include/global.h"

class zipf{
    public:
        zipf(int n,double ratio);
    public:
        int getZipfDistribution(vector<double> & distribution);
        int printZipf();
        int printZipfInFile(string pathname);
    private:
        int zipfDistribution();
    private:
        unsigned int   elements;
        double zipfRatio;
        vector<double> result;
};
