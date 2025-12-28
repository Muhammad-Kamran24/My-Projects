#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <sstream>
#include <iomanip>

using namespace std;

#define MAX_CANDIDATES 10
#define MAX_ELECTIONS 5
#define MAX_VOTERS 100

const string ADMIN_USERNAME = "admin";
const string ADMIN_PASSWORD = "admin123";
const string VOTER_FILE = "voters.csv";
const string ELECTION_FILE = "elections.csv";
const string CANDIDATE_FILE = "candidates.csv";

bool strongPassword(const string& password)
{
	if (password.length() < 8)
	{
		cout << "Password must be at least 8 characters long.\n";
		return false;
	}
	bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
	for (char ch : password)
	{
		if (isupper(ch)) hasUpper = true;
		else if (islower(ch)) hasLower = true;
		else if (isdigit(ch)) hasDigit = true;
		else if (ispunct(ch)) hasSpecial = true;
	}
	if (!hasUpper || !hasLower || !hasDigit || !hasSpecial)
	{
		cout << "Password must contain at least one uppercase letter, one lowercase letter, one digit, and one special character.\n";
		return false;
	}
	return true;
}

bool isAllDigits(const string& str)
{
    for (char ch : str)
    {
        if (!isdigit(ch))
        {
            cout << "Invalid input. Only digits are allowed.\n";
            return false;
        }
    }
    return true;
}

bool isAllAlpha(const string& str)
{
    for (int i = 0; i < str.length(); i++)
    {
        char ch = str[i];
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')))
        {
            cout << "Invalid input. Only alphabets are allowed.\n";
            return false;
        }
    }
    return true;
}


int generateVoterID()
{
    const string idFile = "voter_id.txt";
    int lastID = 1000;

    ifstream in(idFile);
    if (in)
    {
        in >> lastID;
        in.close();
    }

    int newID = lastID + 1;

    ofstream out(idFile);
    if (out)
    {
        out << newID;
        out.close();
    }

    return newID;
}

int generateCandidateID()
{
    const string idFile = "candidate_id.txt";
    int lastID = 0;
    ifstream in(idFile);
    if (in)
    {
        in >> lastID;
        in.close();
    }
    int newID = lastID + 1;
    ofstream out(idFile);
    if (out)
    {
        out << newID;
        out.close();
    }
    return newID;
}


class User
{
protected:
    string username;
    string password;

public:
    User() {}
    User(const string& uname, const string& pass) : username(uname), password(pass) {}

    virtual bool login(const string& u, const string& p)
    {
        return username == u && password == p;
    }

    virtual string getRole() const = 0;
    string getUsername() const { return username; }
};

class Voter : public User
{
private:
    int voterID;
    bool hasVoted[MAX_ELECTIONS];

public:
    Voter(const string& user = "", const string& pass = "")
    {
        username = user;
        password = pass;
        voterID = 0;
        for (int i = 0; i < MAX_ELECTIONS; i++)
        {
            hasVoted[i] = false;
        }
    }


    bool login(const string& u, const string& p) override
    {
        return (username == u && password == p);
    }

    string getRole() const override
    {
        return "voter";
    }

    bool hasAlreadyVoted(int electionIndex) const
    {
        if (electionIndex >= 0 && electionIndex < MAX_ELECTIONS)
            return hasVoted[electionIndex];
        return false;
    }

    void viewVotingStatus()
    {
        cout << "\n+-----------  Voting Status  -----------+  \n";
        for (int i = 0; i < MAX_ELECTIONS; ++i)
        {
            cout << "Election " << (i + 1) << ": ";
            if (hasVoted[i])
                cout << "Voted\n";
            else
                cout << "Not Voted\n";
        }
    }

    void markVoted(int electionIndex)
    {
        if (electionIndex >= 0 && electionIndex < MAX_ELECTIONS)
            hasVoted[electionIndex] = true;
    }

    int getVoterID() const
    {
        return voterID;
    }

    void setVoterID(int id)
    {
        voterID = id;
    }


    void display() const
    {
        cout << "| Voter Username: " << username << " || ID: " << voterID << " |" << endl;
    }

    void saveToFile()
    {
        const char* TEMP_FILE = "temp_voters.csv";

        ifstream in(VOTER_FILE);
        ofstream out(TEMP_FILE);

        if (!out)
        {
            cout << "Error creating temporary file.\n";
            return;
        }

        bool updated = false;
        string line;

        while (getline(in, line))
        {
            stringstream ss(line);
            string fileUsername, filePassword, fileVoterID;
            getline(ss, fileUsername, ',');
            getline(ss, filePassword, ',');
            getline(ss, fileVoterID, ',');

            if (fileUsername == username)
            {
                out << username << "," << password << "," << voterID;
                for (int i = 0; i < MAX_ELECTIONS; ++i)
                    out << "," << (hasVoted[i] ? "1" : "0");
                out << "\n";
                updated = true;
            }
            else
            {
                out << line << "\n";
            }
        }

        in.close();

        if (!updated)
        {
            out << username << "," << password << "," << voterID;
            for (int i = 0; i < MAX_ELECTIONS; ++i)
                out << "," << (hasVoted[i] ? "1" : "0");
            out << "\n";
        }

        out.close();

        ifstream tempIn(TEMP_FILE);
        ofstream finalOut(VOTER_FILE);

        if (!finalOut)
        {
            cout << "Error writing to final voter file.\n";
            return;
        }

        while (getline(tempIn, line))
        {
            finalOut << line << "\n";
        }

        tempIn.close();
        finalOut.close();
    }




    void loadFromFile(string uname)
    {
        ifstream in(VOTER_FILE);
        if (!in)
        {
            cout << "Error opening voters.csv\n";
            return;
        }

        string line;
        while (getline(in, line))
        {
            stringstream ss(line);
            string user, pass, token;
            int id;
            bool voted[MAX_ELECTIONS] = { false };

            // Parse fields
            getline(ss, user, ',');
            getline(ss, pass, ',');
            getline(ss, token, ',');
            id = stoi(token);

            for (int i = 0; i < MAX_ELECTIONS && getline(ss, token, ','); ++i)
            {
                voted[i] = (token == "1");
            }

            if (user == uname)
            {
                username = user;
                password = pass;
                voterID = id;
                for (int i = 0; i < MAX_ELECTIONS; ++i)
                {
                    hasVoted[i] = voted[i];
                }
                break;
            }
        }

        in.close();
    }


};


class Candidate
{
private:
    int id;
    string name;
    string party;
    int votes;

public:

    Candidate() : id(0), name(""), party(""), votes(0) {}
    Candidate(int newID, string name, string party) : name(name), party(party), votes(0), id(newID) {}

    int getId() const { return id; }
    void setId(int id) { this->id = id; }
    string getName() const { return name; }
    string getParty() const { return party; }
    int getVotes() const { return votes; }
    void setVotes(int v) { votes = v; }

    void incrementVotes() { votes++; }

    void displayInfo() const
    {
        cout << "ID: " << id << " || Name: " << name << " || Party: " << party
            << " || Votes: " << votes << endl;
    }

    void updateCandidate(const string& electionTitle, const string& candidateName, int newVoteCount)
    {
        const char* TEMP_FILE = "temp_candidates.csv";

        ifstream in(CANDIDATE_FILE);
        ofstream out(TEMP_FILE);

        if (!out)
        {
            cout << "Error creating temporary candidate file.\n";
            return;
        }

        string line;
        bool updated = false;

        getline(in, line);
        out << line << "\n";

        while (getline(in, line))
        {
            stringstream ss(line);
            string fileId, fileTitle, fileName, party, voteStr;
            getline(ss, fileId, ',');
            getline(ss, fileTitle, ',');
            getline(ss, fileName, ',');
            getline(ss, party, ',');
            getline(ss, voteStr, ',');

            if (fileTitle == electionTitle && fileName == candidateName)
            {
                out << fileId << "," << fileTitle << "," << fileName << "," << party << "," << newVoteCount << "\n";
                updated = true;
            }
            else
            {
                out << line << "\n";
            }
        }

        in.close();

        if (!updated)
        {

            out << id << "," << electionTitle << "," << candidateName << "," << party << "," << newVoteCount << "\n";
        }

        out.close();

        ifstream tempIn(TEMP_FILE);
        ofstream finalOut(CANDIDATE_FILE);

        if (!finalOut)
        {
            cout << "Error opening final candidate file for writing.\n";
            return;
        }

        while (getline(tempIn, line))
        {
            finalOut << line << "\n";
        }

        tempIn.close();
        finalOut.close();
    }


    void viewAllCandidates(const string& electionTitle)
    {

        cout << "\n+-----------------  Registered Candidates for Election: " << electionTitle << "  --------------------+\n";

        ifstream fin(CANDIDATE_FILE);
        if (!fin)
        {
            cout << "No Candidates are Registered Yet.\n";
            return;
        }

        string line;
        getline(fin, line);

        bool anyFound = false;
        cout << "\n+========================================================================+\n";
        while (getline(fin, line))
        {
            stringstream ss(line);
            string id, title, name, party, votes;

            getline(ss, id, ',');
            getline(ss, title, ',');
            getline(ss, name, ',');
            getline(ss, party, ',');
            getline(ss, votes, ',');

            if (title == electionTitle)
            {
                cout << "| " << left << setw(10) << ("ID: " + id)
                    << "| " << left << setw(20) << ("Name: " + name)
                    << "| " << left << setw(20) << ("Party: " + party)
                    << "| " << left << setw(15) << ("Votes: " + votes)
                    << "|\n";
                anyFound = true;
            }
        }
        cout << "+========================================================================+\n";

        fin.close();

        if (!anyFound)
        {
            cout << "No candidates registered for this election.\n";
        }
    }
};



class Election
{
protected:
    string title;
    string province;
    string type;
    Candidate* candidates[MAX_CANDIDATES] = { nullptr };
    int candidateCount;
    bool isActive;

public:
    Election(string title, string type, string pro) : title(title), type(type), province(pro), candidateCount(0) {}

    virtual void displayCandidates() const = 0;
    virtual void vote(int candidateId) = 0;
    virtual void showResults(const string& electionTitle) const = 0;
    virtual void loadAllCandidates() = 0;
    string getTitle() const { return title; }
    string getType() const { return type; }
    string getProvince() const { return province; }

    void setTitle(const string& newTitle) { title = newTitle; }
    int getCandidateCount() const { return candidateCount; }

    Candidate* getCandidate(int index)
    {
        if (index >= 0 && index < candidateCount)
        {
            return candidates[index];
        }
        return nullptr;
    }

    bool addCandidate(Candidate* candidate)
    {
        if (candidateCount < MAX_CANDIDATES)
        {
            candidates[candidateCount++] = candidate;
            return true;
        }
        cout << "Cannot add more candidates. Maximum limit reached.\n";
        return false;
    }


    bool isEligibleToVote(int voterAge, const string& voterProvince, const string& cnic)
    {
        if (voterAge < 18 || voterProvince != province)
        {
            return false;
        }
        return true;
    }

    void cleanupCandidates()
    {
        for (int i = 0; i < candidateCount; i++)
        {
            delete candidates[i];
            candidates[i] = nullptr;
        }
        candidateCount = 0;
    }

};





class LocalElection : public Election
{
public:
    LocalElection(string title, string pro) : Election(title, "local", pro) {}


    void displayCandidates() const override
    {
        cout << "\nLocal Election: " << title << "\n";
        cout << "==================================\n";
        candidates[0]->viewAllCandidates(title);
    }

    void loadAllCandidates() override
    {
        ifstream in(CANDIDATE_FILE);
        if (!in)
        {
            cout << "Failed to open candidates file.\n";
            return;
        }

        string line;
        getline(in, line);

        while (getline(in, line) && candidateCount < MAX_CANDIDATES)
        {
            stringstream ss(line);
            string idStr, title, name, party, votesStr;
            int votes, id;

            getline(ss, idStr, ',');
            getline(ss, title, ',');
            getline(ss, name, ',');
            getline(ss, party, ',');
            getline(ss, votesStr, ',');
            votes = stoi(votesStr);
            id = stoi(idStr);

            Candidate* candidate = new Candidate(id, name, party);
            candidate->setVotes(votes);
            candidates[candidateCount++] = candidate;
        }

        in.close();
    }


    void vote(int candidateId) override
    {
        bool validVote = false;
        for (int i = 0; i < candidateCount; i++)
        {
            if (candidates[i]->getId() == candidateId)
            {
                cout << "candidate id: " << candidates[i]->getId() << endl;
                candidates[i]->incrementVotes();
                candidates[i]->updateCandidate(title, candidates[i]->getName(), candidates[i]->getVotes());
                validVote = true;
                cout << "Vote successfully cast for candidate: " << candidates[i]->getName() << endl;
                break;
            }
        }

        if (!validVote)
        {
            cout << "Invalid candidate ID." << endl;
        }
    }

    void showResults(const string& electionTitle) const override
    {
        cout << "\nResults for Local Election: " << title << " \n";
        cout << "=========================================\n";

        candidates[0]->viewAllCandidates(electionTitle);

        cout << "\n+----------  Total Votes:  -----------+\n";
        int totalVotes = 0;
        for (int i = 0; i < candidateCount; i++)
        {
            totalVotes += candidates[i]->getVotes();
        }
        cout << "Total Votes: " << totalVotes << endl;

        cout << "\n";
        cout << "              +------------+\n";
        cout << "              |   WINNER   | \n";
        cout << "              +------------+\n";

        int maxVotes = 0;
        string winnerName, winnerParty;
        for (int i = 0; i < candidateCount; i++)
        {
            if (candidates[i]->getVotes() > maxVotes)
            {
                maxVotes = candidates[i]->getVotes();
                winnerName = candidates[i]->getName();
                winnerParty = candidates[i]->getParty();
            }
        }

        if (maxVotes > 0)
        {
            cout << winnerName << " from " << winnerParty << " party has *WON* the  Local Elections " << electionTitle << " with " << maxVotes << " votes.\n";
        }
        else
        {
            cout << "No votes cast yet.\n";
        }
    }
};



class NationalElection : public Election
{
public:
    NationalElection(string title, string pro) : Election(title, "national", pro) {}


    void displayCandidates() const override
    {
        cout << "\nNational Election: " << title << "\n";
        cout << "====================================\n";
        candidates[0]->viewAllCandidates(title);
    }

    void loadAllCandidates() override
    {
        ifstream in(CANDIDATE_FILE);
        if (!in)
        {
            cout << "Failed to open candidates file.\n";
            return;
        }

        string line;
        getline(in, line);

        while (getline(in, line) && candidateCount < MAX_CANDIDATES)
        {
            stringstream ss(line);
            string idStr, title, name, party, votesStr;
            int votes, id;

            getline(ss, idStr, ',');
            getline(ss, title, ',');
            getline(ss, name, ',');
            getline(ss, party, ',');
            getline(ss, votesStr, ',');
            votes = stoi(votesStr);
            id = stoi(idStr);

            Candidate* candidate = new Candidate(id, name, party);
            candidate->setVotes(votes);
            candidates[candidateCount++] = candidate;
        }

        in.close();
    }

    void vote(int candidateId) override
    {
        bool validVote = false;
        for (int i = 0; i < candidateCount; i++)
        {
            if (candidates[i]->getId() == candidateId)
            {
                candidates[i]->incrementVotes();
                candidates[i]->updateCandidate(title, candidates[i]->getName(), candidates[i]->getVotes());
                validVote = true;
                cout << "Vote successfully cast for candidate: " << candidates[i]->getName() << endl;
                break;
            }
        }

        if (!validVote)
        {
            cout << "Invalid candidate ID !" << endl;
        }
    }

    void showResults(const string& electionTitle) const override
    {
        cout << "\nResults for National Election: " << title << "\n";
        cout << "=================================================\n";
        candidates[0]->viewAllCandidates(electionTitle);

        cout << "\n+-------  Total Votes:  -------+\n";
        int totalVotes = 0;
        for (int i = 0; i < candidateCount; i++)
        {
            totalVotes += candidates[i]->getVotes();
        }
        cout << "Total Votes: " << totalVotes << endl;
        if (totalVotes != 0)
        {
            cout << "\n";
            cout << "             +------------+\n";
            cout << "             |   WINNER   |\n";
            cout << "             +------------+\n";
        }
        int maxVotes = 0;
        string winnerName, winnerParty;
        for (int i = 0; i < candidateCount; i++)
        {
            if (candidates[i]->getVotes() > maxVotes)
            {
                maxVotes = candidates[i]->getVotes();
                winnerName = candidates[i]->getName();
                winnerParty = candidates[i]->getParty();
            }
        }

        if (maxVotes > 0)
        {
            cout << winnerName << " from " << winnerParty << " party has *WON* the  National Elections " << electionTitle << " with " << maxVotes << " votes.\n";
        }
        else
        {
            cout << "No votes cast yet.\n";
        }
    }
};



class Administrator : public User
{
private:
    Election* elections[MAX_ELECTIONS] = { nullptr };
    int electionCount;

public:
    Administrator() : electionCount(0) {}
    Administrator(const string& user, const string& pass) : User(user, pass), electionCount(0) {}

    bool login(const string& user, const string& pass) override
    {
        return (username == user && password == pass);
    }

    string getRole() const override
    {
        return "admin";
    }

    void displayInfo() const
    {
        cout << "\n+-------- Administrator Info  --------+\n";
        cout << "Username: " << username << endl;
    }

    void addCandidate(int id, const string& electionTitle, const string& name, const string& party)
    {
        string filename = CANDIDATE_FILE;
        bool fileIsEmpty = false;

        ifstream checkFile(filename);
        if (!checkFile || checkFile.peek() == ifstream::traits_type::eof())
        {
            fileIsEmpty = true;
        }
        checkFile.close();

        ofstream out(filename, ios::app);
        if (out.is_open())
        {
            if (fileIsEmpty)
            {
                out << "ID,ElectionTitle,Name,Party,Votes\n";
            }

            out << id << "," << electionTitle << "," << name << "," << party << "," << 0 << "\n";
            out.close();
        }
        else
        {
            cout << "Error adding candidate.\n";
        }
    }


    void viewAllVoters()
    {
        cout << "\n+-------------  Registered Voters  --------------+\n";
        ifstream fin(VOTER_FILE);
        if (!fin)
        {
            cout << "No Registered Voters! Yet.\n";
            return;
        }
        string line;
        getline(fin, line);
        cout << "\n+=================================================================+\n";
        while (getline(fin, line))
        {
            stringstream ss(line);
            string name, pass, id, votedStatus;
            getline(ss, name, ',');
            getline(ss, pass, ',');
            getline(ss, id, ',');
            getline(ss, votedStatus, ',');
            cout << "| " << left << setw(20) << ("Name: " + name)
                << "| " << left << setw(20) << ("ID: " + id)
                << "| " << left << setw(20) << ("Voted Status: " + votedStatus)
                << "|\n";
        }
        cout << "+=================================================================+\n";
        fin.close();
    }

};


string trim(const string& str)
{
    size_t start = 0;
    while (start < str.length() && isspace(str[start])) ++start;

    size_t end = str.length();
    while (end > start && isspace(str[end - 1])) --end;

    return str.substr(start, end - start);
}

string toLower(const string& str)
{
    string lowerStr = str;
    for (size_t i = 0; i < lowerStr.length(); ++i)
        lowerStr[i] = tolower(lowerStr[i]);
    return lowerStr;
}

bool isValidCNIC(const string& cnic)
{
    if (cnic.length() != 13)
        return false;

    for (int i = 0; i < 13; ++i)
    {
        if (cnic[i] < '0' || cnic[i] > '9')
        {
            cout << "Invalid CNIC format. CNIC should be 13 digits.\n";
            return false;
        }
    }
    return true;
}

class ElectionManager
{
private:
    Voter voter;
    Candidate candidate;
    Administrator admin;
    Election* elections[MAX_ELECTIONS] = { nullptr };
    int electionCount;

public:
    ElectionManager() : electionCount(0) {}

    bool createLocalElection(string title, string province)
    {
        if (electionCount < MAX_ELECTIONS)
        {
            elections[electionCount++] = new LocalElection(title, province);
            return true;
        }
        else
        {
            cout << "Error: Maximum number of elections reached. Cannot create more elections.\n";
            return false;
        }
    }

    bool createNationalElection(string title, string province)
    {
        if (electionCount < MAX_ELECTIONS)
        {
            elections[electionCount++] = new NationalElection(title, province);
            return true;
        }
        else
        {
            cout << "Error: Maximum number of elections reached. Cannot create more elections.\n";
            return false;
        }
    }

    int getElectionCount() const
    {
        return electionCount;
    }

    bool displayElections() const
    {
        if (electionCount == 0)
        {
            cout << electionCount << endl;
            cout << "No elections available.\n";
            return false;
        }
        cout << "Elections:\n";
        for (int i = 0; i < electionCount; i++)
        {
            cout << i + 1 << ". " << elections[i]->getTitle() << " (" << elections[i]->getType() << ")" << endl;
        }
        return true;
    }

    bool addCandidatesToElection(int electionIndex, Candidate* candidates[], int numCandidates)
    {
        string electionTitle = elections[electionIndex]->getTitle();

        for (int i = 0; i < numCandidates; i++)
        {
            int newID = generateCandidateID();
            candidates[i]->setId(newID);

            if (!elections[electionIndex]->addCandidate(candidates[i]))
            {
                cout << "Failed to add candidate: " << candidates[i]->getName() << endl;
                return false;
            }

            admin.addCandidate(newID, electionTitle, candidates[i]->getName(), candidates[i]->getParty());
        }

        cout << "Candidates added successfully to election: " << elections[electionIndex]->getTitle() << endl;
        return true;
    }



    void viewCandidate(int electionIndex)
    {
        string electionTitle = elections[electionIndex]->getTitle();
        candidate.viewAllCandidates(electionTitle);
    }

    void loadAllCandidates()
    {
        for (int i = 0; i < electionCount; i++)
        {
            if (elections[i])
            {
                elections[i]->loadAllCandidates();
            }
        }
    }

    void cleanupAllCandidates()
    {
        for (int i = 0; i < electionCount; i++)
        {
            if (elections[i])
            {
                elections[i]->cleanupCandidates();
            }
        }
    }


    bool voteInElection(int electionIndex)
    {
        string province, cnic;
        int age, candidateId;

        if (elections[electionIndex]->getCandidateCount() == 0)
        {
            cout << "No candidates available for this election.\n";
            return false;
        }

        do
        {
            cout << "Enter your Province: ";
            getline(cin, province);
        } while (!isAllAlpha(province));

        cout << "Enter your age: ";
        cin >> age;
        cin.ignore();

        do {
            cout << "Enter your CNIC: ";
            getline(cin, cnic);
            cin.ignore();
        } while (!isValidCNIC(cnic));

        if (elections[electionIndex]->isEligibleToVote(age, province, cnic))
        {
            cout << "You are eligible to vote.\n";
        }
        else
        {
            cout << "You are not eligible to vote in this election !\n";
            return false;
        }

        elections[electionIndex]->displayCandidates();

        cout << "Enter Candidate ID to vote for: ";
        cin >> candidateId;
        cin.ignore();

        elections[electionIndex]->vote(candidateId);
        cout << "Vote cast successfully for candidate ID: " << candidateId << endl;
        return true;
    }

    void showResults(int electionIndex) const
    {
        string electionTitle = elections[electionIndex]->getTitle();
        elections[electionIndex]->showResults(electionTitle);
    }

    void saveElectionsToFile()
    {
        ofstream out(ELECTION_FILE);
        if (!out)
        {
            cout << "Failed to open elections file for writing.\n";
            return;
        }

        out << "ElectionTitle,Type,Province\n";

        for (int i = 0; i < electionCount; ++i)
        {
            out << elections[i]->getTitle() << ","
                << elections[i]->getType() << ","
                << elections[i]->getProvince() << "\n";
        }

        out.close();
        cout << "Elections saved to file.\n";
    }


    void loadElectionsFromFile()
    {
        ifstream in(ELECTION_FILE);
        if (!in)
        {
            cout << "Failed to open elections file.\n";
            return;
        }

        string line;
        getline(in, line);

        while (getline(in, line) && electionCount < MAX_ELECTIONS)
        {
            stringstream ss(line);
            string title, type, province;
            getline(ss, title, ',');
            getline(ss, type, ',');
            getline(ss, province, ',');

            title = trim(title);
            type = toLower(trim(type));
            province = trim(province);

            if (type == "local")
            {
                createLocalElection(title, province);
            }
            else if (type == "national")
            {
                createNationalElection(title, province);
            }
            else
            {
                cout << "Unknown election type: " << type << "\n";
            }

        }

        in.close();
    }

    ~ElectionManager()
    {
        for (int i = 0; i < electionCount; i++)
        {
            delete elections[i];
        }
    }
};


void showVoterMenu(Voter& voter, ElectionManager& electionManager)
{
    int choice;

    while (true)
    {
        cout << "\n+---------------------------------------------------------------------------------------------------------------+\n\n";
        cout << "\n+===============================================+\n";
        cout << "|                Voter Menu                     |\n";
        cout << "+===============================================+\n";
        cout << "|   1. View Available Elections                 |\n";
        cout << "|   2. Vote in an Election                      |\n";
        cout << "|   3. View Voting Status                       |\n";
        cout << "|   4. Return To Menu                           |\n";
        cout << "+===============================================+\n";
        cout << "Enter your choice (1-4):  ";
        cin >> choice;

        cout << "+---------------------------------------------------------------------------------------------------------------+\n";

        switch (choice)
        {
        case 1:
            electionManager.displayElections();
            break;
        case 2:
        {
            if (electionManager.displayElections())
            {

                int electionIndex;
                cout << "Enter election index to vote: ";
                cin >> electionIndex;
                cin.ignore();

                if (electionIndex < 0 || electionIndex >= MAX_ELECTIONS)
                {
                    cout << "Invalid election index.\n";
                    break;
                }
                else
                {
                    if (voter.hasAlreadyVoted(electionIndex - 1))
                    {
                        cout << "You have already voted in this election.\n";
                    }
                    else
                    {
                        if (electionManager.voteInElection(electionIndex - 1))
                        {
                            voter.markVoted(electionIndex - 1);
                            voter.saveToFile();
                            cout << "Vote cast successfully!\n";
                        }
                        else
                        {
                            cout << "Failed to cast vote.\n";
                        }
                    }
                }
            }
            break;
        }

        case 3:
            voter.viewVotingStatus();
            break;
        case 4:
            cout << "Returning to Main Menu...\n\n";
            return;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

void showAdminMenu(Administrator& admin, ElectionManager& electionManager)
{
    int choice;
    int electionIndex;
    while (true)
    {
        cout << "\n+---------------------------------------------------------------------------------------------------------------+\n\n";
        cout << "\n+===============================================+\n";
        cout << "|               Administrator Menu              |\n";
        cout << "+===============================================+\n";
        cout << "|   1. Create Local Election                    |\n";
        cout << "|   2. Create National Election                 |\n";
        cout << "|   3. Add Candidates to Election               |\n";
        cout << "|   4. View All Voters                          |\n";
        cout << "|   5. View All Candidates                      |\n";
        cout << "|   6. View Election Results                    |\n";
        cout << "|   7. Save and Return to Main Menu.            |\n";
        cout << "+===============================================+\n";

        cout << "Enter your choice (1-7):  ";
        cin >> choice;
        cin.ignore();

        cout << "+---------------------------------------------------------------------------------------------------------------+\n";

        switch (choice)
        {
        case 1:
        {
            string title, province;
            do {
                cout << "Enter title for Local Election : ";
                getline(cin, title);
            } while (!isAllAlpha(title));

            do {
                cout << "Enter province in which Local Election is to be Started: ";
                getline(cin, province);
            } while (!isAllAlpha(province));

            if (electionManager.createLocalElection(title, province))
            {
                cout << "Local Election " << title << " created successfully.\n";
            }
            else
            {
                cout << "Failed to create Local Election.\n";
            }
            break;
        }
        case 2:
        {
            string title, province;
            do {
                cout << "Enter title for National Election : ";
                getline(cin, title);
            } while (!isAllAlpha(title));
            do {
                cout << "Enter province in which National Election is to be Started: ";
                getline(cin, province);
            } while (!isAllAlpha(province));

            if (electionManager.createNationalElection(title, province))
            {
                cout << "National Election " << title << " created successfully.\n";
            }
            else
            {
                cout << "Failed to create National Election.\n";
            }
            break;
        }
        case 3:
        {
            if (electionManager.displayElections())
            {
                cout << "Enter election index to add candidates: ";
                cin >> electionIndex;
                cin.ignore();
                if (electionIndex < 0 || electionIndex > electionManager.getElectionCount())
                {
                    cout << "Invalid election index.\n";
                    break;
                }
                else
                {
                    int numCandidates;
                    cout << "Enter number of candidates: ";
                    cin >> numCandidates;
                    cin.ignore();

                    Candidate* candidates[MAX_CANDIDATES];
                    for (int i = 0; i < numCandidates; i++)
                    {
                        string name, party;
                        cout << "Candidate " << i + 1 << " Name: ";
                        getline(cin, name);
                        cout << "Party: ";
                        getline(cin, party);
                        candidates[i] = new Candidate(generateCandidateID(), name, party);
                    }

                    electionManager.addCandidatesToElection(electionIndex - 1, candidates, numCandidates);

                    for (int i = 0; i < numCandidates; i++)
                    {
                        delete candidates[i];
                    }
                }
            }
            break;
        }
        case 4:
            admin.viewAllVoters();
            break;

        case 5:
            if (electionManager.displayElections())
            {
                cout << "Enter election index to view candidates: ";
                cin >> electionIndex;
                cin.ignore();
                if (electionIndex < 0 || electionIndex > electionManager.getElectionCount())
                {
                    cout << "Invalid election index.\n";
                    break;
                }
                else
                {
                    electionManager.viewCandidate(electionIndex - 1);
                }
            }
            break;
        case 6:
            if (electionManager.displayElections())
            {
                cout << "Enter election index to view results: ";
                cin >> electionIndex;
                cin.ignore();

                if (electionIndex < 0 || electionIndex > electionManager.getElectionCount())
                {
                    cout << "Invalid election index.\n";
                    break;
                }
                else
                {
                    electionManager.showResults(electionIndex - 1);
                }
            }
            break;
        case 7:
            electionManager.saveElectionsToFile();
            cout << "Returning to Main Menu...\n\n";
            return;
        default:
            cout << "Invalid choice! Try Again.\n";
        }
    }
}

int main()
{
    ElectionManager electionManager;
    electionManager.loadElectionsFromFile();
    electionManager.loadAllCandidates();
    cout << "                                  +---------------------------------------------------------+\n";
    cout << "                                  |   Welcome to Pakistan's Online Voting System (POVS)     |\n";
    cout << "                                  +---------------------------------------------------------+\n";

    int roleChoice;
    do
    {
        cout << "\n+===================================+\n";
        cout << "|             Main Menu             |\n";
        cout << "+===================================+\n";
        cout << "| 1. Voter                          |\n";
        cout << "| 2. Administrator                  |\n";
        cout << "| 3. Exit                           |\n";
        cout << "+===================================+\n";
        cout << "Enter your choice (1-3): ";
        cin >> roleChoice;
        cin.ignore();
        cout << "+---------------------------------------------------------------------------------------------------------------+\n";
        if (roleChoice == 1)
        {
            string uname, pass;
            bool loggedIn = false;

            while (!loggedIn)
            {
                cout << "\n=======  Voter Registration/Login  =======\n";
                cout << "Enter Voter Username: ";
                getline(cin, uname);
                do {
                    cout << "Enter Voter Password: ";
                    getline(cin, pass);
				} while (!strongPassword(pass));
                bool userExists = false;
                ifstream in(VOTER_FILE);
                string fileUser, filePass;
                int voterID;

                string line;
                while (getline(in, line))
                {
                    stringstream ss(line);
                    getline(ss, fileUser, ',');
                    getline(ss, filePass, ',');
                    getline(ss, line, ',');
                    voterID = stoi(line);

                    if (fileUser == uname)
                    {
                        userExists = true;
                        if (filePass == pass)
                        {
                            cout << "Welcome back, " << uname << "!\n";
                            Voter voter(uname, pass);
                            voter.setVoterID(voterID);
                            voter.loadFromFile(uname);
                            showVoterMenu(voter, electionManager);
                            loggedIn = true;
                            break;
                        }
                        else
                        {
                            cout << "Incorrect Username or Password! Please try again.\n";
                            break;
                        }
                    }
                }
                in.close();

                if (!userExists && !loggedIn)
                {
                    cout << "New Voter Registration\n";
                    int newID = generateVoterID();
                    Voter voter(uname, pass);
                    voter.setVoterID(newID);
                    voter.saveToFile();
                    cout << "Registration successful! Welcome, " << uname << "\n";

                    showVoterMenu(voter, electionManager);
                    loggedIn = true;
                }
            }
        }
        else if (roleChoice == 2)
        {
            string uname, pass;
            cout << "\n=========  ADMIN LOGIN  ==========\n";
            cout << "Enter admin username: ";
            getline(cin, uname);
            cout << "Enter admin password: ";
            getline(cin, pass);

            if (uname == ADMIN_USERNAME && pass == ADMIN_PASSWORD)
            {
                cout << "Welcome back, Administrator!\n";
                Administrator admin(uname, pass);
                showAdminMenu(admin, electionManager);
            }
            else
            {
                cout << "Incorrect admin credentials. Access denied.\n";
            }
        }
        else if (roleChoice == 3)
        {
            cout << "Exiting the program.\n";
            return 0;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    } while (roleChoice != 3);
    electionManager.cleanupAllCandidates();

    return 0;
}