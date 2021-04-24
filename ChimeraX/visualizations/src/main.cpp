#include <iostream>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "MRC.h"
#include <vector>
#include <sstream>
#include<io.h>
#include<list>
#include<direct.h>
#include<fstream>
#include<dirent.h>


using namespace std;

string GetPathDir(string filePath)
{
    string dirPath = filePath;
    size_t p = filePath.find_last_of("\\");
    if (p != -1)
    {
        dirPath.erase(p);
    }
    return dirPath;
}
// create directories
void CreateMultiLevel(string dir)
{
    if (_access(dir.c_str(), 00) == 0)
    {
        return;
    }

    list <string> dirList;
    dirList.push_front(dir);

    string curDir = GetPathDir(dir);
    while (curDir != dir)
    {
        if (_access(curDir.c_str(), 00) == 0)
        {
            break;
        }

        dirList.push_front(curDir);

        dir = curDir;
        curDir = GetPathDir(dir);
    }

    for (auto it : dirList)
    {
        _mkdir(it.c_str());
    }
}

vector<string> split (const string &s, char delim) {
    vector<string> result;
    stringstream ss (s);
    string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

int main(int argc, const char **argv) 
{
    ofstream fout("pre_visua.txt");
    fout << "Start implement mrc files..." << endl;

    if(argc < 7)
    {
        fout << "Error: Not enough args" << endl;
        cout << "ERROR: Not enough args" << endl;
        return 0;
    }

    string mrcFile = argv[1];
    string pdbFile = argv[2];
    string chain = argv[3];
    string predictionFilePath = argv[4];
    string outputDirectoryPath = argv[5];
    string mapsFolderPath = argv[6];

    fout << "Argument: " << mrcFile << "-" << pdbFile << "-" << chain << endl;
    fout << "predictionFilePath: " << predictionFilePath << endl;
    fout << "outputDirectoryPath: " << outputDirectoryPath << endl;
    fout << "mapsFolderPath: " << mapsFolderPath << endl;

    string prefix = "_pred_";

    bool trueLabels = false;

    if(argc > 7)
    {
        prefix = "_true_";
        trueLabels = true;
    }


    outputDirectoryPath = outputDirectoryPath + "\\" + mrcFile + "_" + pdbFile + "\\";
    //mkdir(outputDirectoryPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    // mkdir(outputDirectoryPath.c_str());
        // Creating a directory
    fout << "Create visualization path..." << endl;
    fout << flush;

    CreateMultiLevel(outputDirectoryPath);

    DIR *dir;
    if ((dir=opendir(outputDirectoryPath.c_str())) == NULL )
    {
        fout << "Output file path can not be created...";
        fout << flush;
        return 0;
    }
    string outputFileHelix = outputDirectoryPath + chain + prefix + "helix.mrc";
    string outputFileStrand = outputDirectoryPath + chain + prefix + "sheet.mrc";
    fout << outputFileHelix << endl;
    fout << "Read 1.0APIX.mrc file..." << endl;
    fout << flush;

    string mrcFilePath = mapsFolderPath + "\\" + mrcFile + "_" + pdbFile + "\\" + chain + "-1.0APIX.mrc";
    fout << "mrcFilePath" << mrcFilePath << endl;
    MRC modifiedMap;
    modifiedMap.EMRead(mrcFilePath);

    //set all voxels to 0
    for(int i = 0; i < modifiedMap.getNx(); i++)
    {
        for(int j = 0; j < modifiedMap.getNy(); j++)
        {
            for(int k = 0; k < modifiedMap.getNz(); k++)
            {
               modifiedMap.cube[i][j][k] = 0;
            }
        }
    }

    fout << "Start to generate Helix.mrc file..." << endl;
    fout << flush;
    //read predictions and adjust new map
    ifstream inFile;

    inFile.open(predictionFilePath.c_str());
    int xCoord = 0;
    int yCoord = 0;
    int zCoord = 0;
    int label = 0;
    int iter = 0;
    while(!inFile.eof())
    {
        if(!trueLabels)
        {
            inFile >> xCoord >> yCoord >> zCoord  >> label;
        }
        else
        {
            string temp;
            inFile >> temp;
            vector<string> temps = split(temp, ',');
            if(temps.size() != 5)
            {
                break;
            }
            xCoord = stoi(temps[0]);
            yCoord = stoi(temps[1]);
            zCoord = stoi(temps[2]);
            label = stoi(temps[4]);
        }
        if(xCoord < modifiedMap.getNx() && yCoord < modifiedMap.getNy() && zCoord < modifiedMap.getNz())
        {
            if(label == 1)
            {
                modifiedMap.cube[xCoord][yCoord][zCoord] = 1;
            }            
            else
            {
                modifiedMap.cube[xCoord][yCoord][zCoord] = 0;
            }
        }
    }
    inFile.close();
    modifiedMap.write(outputFileHelix);

    for(int i = 0; i < modifiedMap.getNx(); i++)
        for(int j = 0; j < modifiedMap.getNy(); j++)
            for(int k = 0; k < modifiedMap.getNz(); k++)
                modifiedMap.cube[i][j][k] = 0;

    fout << "Start to generate sheet.mrc file..." << endl;
    fout << flush;
    //For Strands
    inFile.open(predictionFilePath.c_str());
    while(!inFile.eof())
    {
        if(!trueLabels)
        {
            inFile >> xCoord >> yCoord >> zCoord  >> label;
        }
        else
        {
            string temp;
            inFile >> temp;
            vector<string> temps = split(temp, ',');
            if(temps.size() != 5)
            {
                break;
            }
            xCoord = stoi(temps[0]);
            yCoord = stoi(temps[1]);
            zCoord = stoi(temps[2]);
            label = stoi(temps[4]);
        }
        if(xCoord < modifiedMap.getNx() && yCoord < modifiedMap.getNy() && zCoord < modifiedMap.getNz())
        {
            if(label == 2)
                modifiedMap.cube[xCoord][yCoord][zCoord] = 1;
            else
                modifiedMap.cube[xCoord][yCoord][zCoord] = 0;
        }
    }
    inFile.close();
    modifiedMap.write(outputFileStrand);
    fout << "DONE..." << endl;
    fout << flush;
    fout.close();
    return 0;
}
