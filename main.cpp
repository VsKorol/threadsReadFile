#include <mutex>
#include <thread>

#include <iostream>
#include <fstream>

#include <algorithm>
#include <string>
#include <regex>

class FileFinder {
    const std::map<char, std::string> specSimbol_ {
        {'?', "."},

        {'(', "\\("},
        {')', "\\)"},
        {'[', "\\["},
        {']', "\\]"},
        {'{', "\\}"},
        {'}', "\\}"},
        {'<', "\\<"},
        {'>', "\\>"},

        {'\\', "\\\\"},
        {'|', "\\|"},
        {'^', "\\^"},
        {'$', "\\$"},
        {'.', "\\."},
        {',', "\\,"},
        {'+', "\\+"},
        {'-', "\\-"},
        {'*', "\\*"},
        {'!', "\\!"},
    };
    void lineHandler();
    void openFile();

public:
    struct FullAnswer {
        int lineNum;
        int colNum;
        std::string matchWord;
    };
    using fullAnswers = std::vector<FullAnswer>;

    FileFinder(int maxTreadNum, const std::string& fileName) :
        maxTreadNum_(maxTreadNum),
        fileName_(fileName)
    {}
    ~FileFinder();

    void find(const std::string& regexStr);
    void setRegexStr(const std::string& regexStr);


    friend std::ostream& operator<< (std::ostream &out, const FileFinder &object);
    friend std::ostream& operator<< (std::ostream &out, const FileFinder::FullAnswer &object);
private:
    int maxTreadNum_;
    std::string fileName_;

    std::mutex muReadFromFile_;
    std::mutex muAddAnswer_;
    std::ifstream file_;
    fullAnswers results_;
    std::string regexStr_;
    int actualNum_;
    std::vector<std::thread> threads_;
};

FileFinder::~FileFinder() {
    if (file_.is_open())
        file_.close();
}

void FileFinder::openFile() {
    file_.open(fileName_, std::ios::in);
    actualNum_ = 1;
}

void FileFinder::setRegexStr(const std::string& regexStr) {
    regexStr_.reserve(regexStr.length());
    for(auto el : regexStr) {
        auto newEl = specSimbol_.find(el);
        if(newEl != specSimbol_.end()) {
            regexStr_.append(newEl->second);
        } else {
           regexStr_.push_back(el);
        }
    }
}

void FileFinder::find(const std::string& regexStr) {
    setRegexStr(regexStr);
    openFile();

    for(int i = 0; i < maxTreadNum_; i++) {
        std::thread thr(&FileFinder::lineHandler, this);
        threads_.emplace_back(std::move(thr));
    }

    for(auto& thr : threads_) {
        thr.join();
    }
    std::cout << *this;

}

void FileFinder::lineHandler() {
    std::string str;
    FullAnswer res;
    std::smatch m;
    std::regex e (regexStr_);

    muReadFromFile_.lock();
    while(!file_.eof()) {
        std::getline(file_, str);
        res.lineNum = actualNum_++;
        muReadFromFile_.unlock();

        if (std::regex_search (str,m,e)) {
            res.matchWord = m[0];
            res.colNum=m.position();

            muAddAnswer_.lock();
            results_.push_back(res);
            muAddAnswer_.unlock();
        }
        muReadFromFile_.lock();
    }
    muReadFromFile_.unlock();

}

std::ostream& operator<< (std::ostream &out, const FileFinder &object) {
    out << std::to_string(object.results_.size()) << std::endl;
    for(auto res : object.results_) {
        out << res <<std::endl;
    }
    return out;
}
std::ostream& operator<< (std::ostream &out, const FileFinder::FullAnswer &object) {
    out << object.lineNum << " "
        << object.colNum << " "
        << object.matchWord;
    return out;
}

int main(int argc, char *argv[])
{
    std::string fileName;
    std::string surStr";
    if(argc < 2){
        return 0;
    }
    fileName = argv[1];
    surStr = argv[2];

    FileFinder ff(1, fileName);
    ff.find(surStr);

    return 1;
}

