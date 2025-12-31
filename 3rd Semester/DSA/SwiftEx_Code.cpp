
#include <iostream>
#include <string>
#include <climits>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <cstdlib>

using namespace std;

// ==========================================
// 1. CONFIGURATION & CONSTANTS
// ==========================================

// ANSI Color Codes for UI Beautification
const string RESET   = "\033[0m";
const string RED     = "\033[31m";
const string GREEN   = "\033[32m";
const string YELLOW  = "\033[33m";
const string BLUE    = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN    = "\033[36m";
const string WHITE   = "\033[37m";
const string BOLD    = "\033[1m";

// Simulation Constraints
// Speed = 10 km/sec to ensure delivery times are reasonable (under 5 mins)
const int SIM_SPEED_KM_PER_SEC = 10;  
const int MAX_CITIES = 100;           
const int HASH_TABLE_SIZE = 100;      
const int MISSING_PARCEL_THRESHOLD = 300; // 300 Seconds limit for missing status

// ==========================================
// 2. UTILITY CLASSES (VALIDATION & UI)
// ==========================================

class UIHelper {
public:
    static void clearScreen() {
        cout << "\033[2J\033[1;1H";
    }

    static void printBanner() {
        cout << CYAN << BOLD << R"(
   _______________________________________________________________
  |                                                               |
  |   _____          _  __ _   ______      _     _                |
  |  / ____|        (_)/ _| | |  ____|    (_)   | |               |
  | | (___ __      ___| |_| |_| |__  __  ___ ___| |               |
  |  \___ \\ \ /\ / / |  _| __|  __| \ \/ / / __| |               |
  |  ____) |\ V  V /| | | | |_| |____ >  <| \__ \ |____           |
  | |_____/  \_/\_/ |_|_|  \__|______/_/\_\_|___/______|          |
  |                                                               |
  |            LOGISTICS INTELLIGENT ENGINE v4.4                  |
  |_______________________________________________________________|
        )" << RESET << endl;
    }

    static void printHeader(string title) {
        cout << endl;
        cout << CYAN << " +-------------------------------------------------------------------------------------------+ " << RESET << endl;
        cout << CYAN << " | " << RESET << BOLD << left << setw(89) << title << RESET << CYAN << " | " << RESET << endl;
        cout << CYAN << " +-------------------------------------------------------------------------------------------+ " << RESET << endl;
    }

    static void printSubHeader(string title) {
        cout << endl;
        cout << BLUE << " ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: " << RESET << endl;
        cout << BLUE << "  >> " << RESET << BOLD << title << RESET << endl;
        cout << BLUE << " ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: " << RESET << endl;
    }

    static void printLine() {
        cout << CYAN << " --------------------------------------------------------------------------------------------- " << RESET << endl;
    }
    
    static void printMenuOption(int id, string text) {
        cout << CYAN << " [" << YELLOW << id << CYAN << "] " << RESET << text << endl;
    }

    static int getIntInput(string prompt, int minLimit = INT_MIN, int maxLimit = INT_MAX) {
        int value;
        while (true) {
            cout << YELLOW << prompt << RESET;
            cin >> value;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
                cout << RED << "    [!] Error: Invalid input. Please enter a valid integer number." << RESET << endl;
            } else if (value < minLimit || value > maxLimit) {
                cout << RED << "    [!] Error: Input out of range. Please enter a value between " << minLimit << " and " << maxLimit << "." << RESET << endl;
            } else {
                cin.ignore(INT_MAX, '\n'); 
                return value;
            }
        }
    }

    static double getDoubleInput(string prompt, double minLimit = 0.0, double maxLimit = 10000.0) {
        double value;
        while (true) {
            cout << YELLOW << prompt << RESET;
            cin >> value;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
                cout << RED << "    [!] Error: Invalid input. Please enter a decimal number." << RESET << endl;
            } else if (value <= minLimit || value > maxLimit) {
                cout << RED << "    [!] Error: Value must be between " << minLimit << " and " << maxLimit << "." << RESET << endl;
            } else {
                cin.ignore(INT_MAX, '\n');
                return value;
            }
        }
    }

    static string getStringInput(string prompt) {
        string value;
        while (true) {
            cout << YELLOW << prompt << RESET;
            getline(cin, value);
            if (!value.empty()) return value;
            cout << RED << "    [!] Error: Input cannot be empty." << RESET << endl;
        }
    }

    static void pressEnterToContinue() {
        cout << "\n " << MAGENTA << "[Press ENTER to Continue...]" << RESET;
        cin.ignore(INT_MAX, '\n');
        cin.get();
    }
    
    static string getCurrentTimeStr() {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char buffer[80];
        strftime(buffer, 80, "%H:%M:%S", ltm);
        return string(buffer);
    }
};

// ==========================================
// 3. CORE DOMAIN OBJECTS
// ==========================================

struct Parcel {
    int id;
    string sourceCity;
    string destCity;
    double weight;
    int priorityLevel; 
    string weightCategory; 
    string status; 
    double shippingCost;
    string assignedRoute;
    int totalDistanceKm;
    time_t creationTime;
    time_t lastUpdateTime;
    time_t dispatchTime;
    int estimatedDurationSec;
    bool isReturning;
    bool willFailOnPath;
    string assignedRiderName;
    string historyLog;

    Parcel() : id(0), weight(0), priorityLevel(3) {}

    Parcel(int pid, string src, string dest, double w, int p) 
        : id(pid), sourceCity(src), destCity(dest), weight(w), priorityLevel(p),
          status("Pickup Queue"), assignedRoute("Not Assigned"), totalDistanceKm(0),
          estimatedDurationSec(0), isReturning(false), willFailOnPath(false), dispatchTime(0), assignedRiderName("None")
    {
        creationTime = time(0);
        lastUpdateTime = time(0);
        
        if (weight <= 50.0) weightCategory = "Light";
        else if (weight <= 150.0) weightCategory = "Heavy";
        else weightCategory = "Fragile";

        shippingCost = 0; 

        addToHistory("Parcel Created at " + src + ". Category: " + weightCategory);
        addToHistory("Placed in " + src + " Pickup Queue.");
    }

    void addToHistory(string event) {
        string timeStr = UIHelper::getCurrentTimeStr();
        historyLog += "    [" + timeStr + "] " + event + "\n";
        lastUpdateTime = time(0);
    }

    string getPriorityStr() {
        if (priorityLevel == 1) return "High";
        if (priorityLevel == 2) return "Med";
        return "Low";
    }
    
    string getStatusColor() {
        if (status == "Delivered") return GREEN;
        if (status == "Delivery Failed" || status == "Returned to Sender" || status == "MISSING") return RED;
        if (status == "In Transit") return BLUE;
        if (status.find("Warehouse") != string::npos) return MAGENTA;
        return YELLOW;
    }

    void displayTableRow() {
        cout << " | " << setw(5) << id 
             << " | " << setw(12) << sourceCity.substr(0,12) 
             << " | " << setw(12) << destCity.substr(0,12)
             << " | " << setw(6) << weight
             << " | " << setw(5) << getPriorityStr()
             << " | " << getStatusColor() << setw(25) << status << RESET
             << " | " << setw(8) << assignedRiderName.substr(0,8)
             << " | " << endl;
    }
    
    void displayFullDetails() {
        UIHelper::printSubHeader("Parcel Details: ID #" + to_string(id));
        cout << " Source:      " << setw(20) << sourceCity << " | Destination: " << destCity << endl;
        cout << " Weight:      " << setw(20) << (to_string(weight) + " kg") << " | Category:    " << weightCategory << endl;
        cout << " Priority:    " << setw(20) << getPriorityStr() << " | Status:      " << getStatusColor() << status << RESET << endl;
        cout << " Route:       " << assignedRoute << endl;
        cout << " Est. Time:   " << estimatedDurationSec << " sec      | Cost:        PKR " << fixed << setprecision(2) << shippingCost << endl;
        cout << " Rider:       " << assignedRiderName << endl;
        
        bool isMissing = (status == "MISSING");
        cout << " Missing?:    " << (isMissing ? (RED + "YES (Confirmed)") : (GREEN + "NO")) << RESET << endl;
        cout << endl << BOLD << " >> HISTORY TIMELINE:" << RESET << endl;
        cout << historyLog << endl;
        UIHelper::printLine();
    }
    
    bool isMissing() {
        if (status == "Delivered" || status == "Delivery Failed" || status == "Returned to Sender" || status == "MISSING") return false;
        time_t now = time(0);
        return (difftime(now, lastUpdateTime) > MISSING_PARCEL_THRESHOLD);
    }
};

struct Rider {
    int id;
    string name;
    string vehicleType;
    double maxLoadCapacity;
    double currentLoad;
    string status; // "Idle", "Busy"

    Rider(int i, string n, string v, double cap)
        : id(i), name(n), vehicleType(v), maxLoadCapacity(cap), currentLoad(0), status("Idle") {}
        
    bool canCarry(double w) {
        return (currentLoad + w <= maxLoadCapacity);
    }
    
    void assignParcel(double w) {
        currentLoad += w;
        status = "Busy";
    }
    
    void reset() {
        currentLoad = 0;
        status = "Idle";
    }
    
    void displayRow() {
        string statColor = (status == "Idle") ? GREEN : RED;
        double util = (currentLoad / maxLoadCapacity) * 100.0;
        cout << " | " << setw(4) << id 
             << " | " << setw(15) << name 
             << " | " << setw(8) << vehicleType
             << " | " << setw(8) << maxLoadCapacity
             << " | " << setw(8) << currentLoad
             << " | " << statColor << setw(10) << status << RESET
             << " | " << fixed << setprecision(1) << setw(5) << util << "%"
             << " |" << endl;
    }
};

// ==========================================
// 4. CUSTOM DATA STRUCTURES
// ==========================================

// --- 4.1 PARCEL LINKED LIST ---
struct ParcelNode {
    Parcel* data;
    ParcelNode* next;
    ParcelNode* prev;
    
    ParcelNode(Parcel* val) : data(val), next(nullptr), prev(nullptr) {}
};

class ParcelList {
public:
    ParcelNode* head;
    ParcelNode* tail;
    int size;

    ParcelList() : head(nullptr), tail(nullptr), size(0) {}

    void pushBack(Parcel* val) {
        ParcelNode* newNode = new ParcelNode(val);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        size++;
    }

    bool remove(Parcel* val) {
        ParcelNode* curr = head;
        while (curr) {
            if (curr->data == val) {
                if (curr == head) {
                    head = head->next;
                    if (head) head->prev = nullptr;
                    else tail = nullptr;
                } else if (curr == tail) {
                    tail = tail->prev;
                    tail->next = nullptr;
                } else {
                    curr->prev->next = curr->next;
                    curr->next->prev = curr->prev;
                }
                delete curr;
                size--;
                return true;
            }
            curr = curr->next;
        }
        return false;
    }
    
    Parcel* popFront() {
        if (!head) return nullptr;
        ParcelNode* temp = head;
        Parcel* val = temp->data;
        head = head->next;
        if (head) head->prev = nullptr;
        else tail = nullptr;
        delete temp;
        size--;
        return val;
    }

    Parcel* popBack() {
        if (!tail) return nullptr;
        ParcelNode* temp = tail;
        Parcel* val = temp->data;
        if (head == tail) {
            head = tail = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete temp;
        size--;
        return val;
    }

    bool isEmpty() { return size == 0; }
};

// --- 4.2 PARCEL QUEUE ---
class ParcelQueue {
private:
    ParcelList list;
public:
    void enqueue(Parcel* val) { list.pushBack(val); }
    Parcel* dequeue() { return list.popFront(); }
    Parcel* peek() { return list.head ? list.head->data : nullptr; }
    bool isEmpty() { return list.isEmpty(); }
    int count() { return list.size; }
    ParcelNode* getHead() { return list.head; }
};

// --- 4.3 TEMPORARY STACK FOR PARCELS ---
class ParcelStack {
private:
    ParcelList list;
public:
    void push(Parcel* val) { list.pushBack(val); }
    Parcel* pop() { return list.popBack(); }
    bool isEmpty() { return list.isEmpty(); }
    void clear() { while(!isEmpty()) pop(); }
};

// --- 4.4 UNDO LOG STACK ---
struct ActionLog {
    string actionType; 
    Parcel* parcelPtr;
    Rider* riderPtr; 
};

struct ActionLogNode {
    ActionLog data;
    ActionLogNode* next;
    ActionLogNode(ActionLog val) : data(val), next(nullptr) {}
};

class UndoStack {
private:
    ActionLogNode* top;
public:
    UndoStack() : top(nullptr) {}
    
    void push(ActionLog val) {
        ActionLogNode* n = new ActionLogNode(val);
        n->next = top;
        top = n;
    }
    
    ActionLog pop() {
        if(!top) return ActionLog{"", nullptr, nullptr};
        ActionLogNode* temp = top;
        ActionLog val = temp->data;
        top = top->next;
        delete temp;
        return val;
    }
    
    bool isEmpty() { return top == nullptr; }
};

// --- 4.5 MIN-HEAP (PRIORITY QUEUE) ---
class ParcelPriorityQueue {
private:
    Parcel* heap[500]; 
    int heapSize;

    bool isHigherPriority(Parcel* p1, Parcel* p2) {
        if (p1->priorityLevel < p2->priorityLevel) return true;
        if (p1->priorityLevel > p2->priorityLevel) return false;
        if (p1->weight > p2->weight) return true;
        if (p1->weight < p2->weight) return false;
        return p1->id < p2->id;
    }

    void heapifyUp(int index) {
        int parent = (index - 1) / 2;
        if (index > 0 && isHigherPriority(heap[index], heap[parent])) {
            swap(heap[index], heap[parent]);
            heapifyUp(parent);
        }
    }

    void heapifyDown(int index) {
        int smallest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < heapSize && isHigherPriority(heap[left], heap[smallest]))
            smallest = left;
        if (right < heapSize && isHigherPriority(heap[right], heap[smallest]))
            smallest = right;

        if (smallest != index) {
            swap(heap[index], heap[smallest]);
            heapifyDown(smallest);
        }
    }

public:
    ParcelPriorityQueue() : heapSize(0) {}

    void insert(Parcel* p) {
        if (heapSize >= 500) {
            cout << RED << " [!] Warehouse Full! Cannot accept new parcels." << RESET << endl;
            return;
        }
        heap[heapSize] = p;
        heapifyUp(heapSize);
        heapSize++;
    }

    Parcel* extractMin() {
        if (heapSize == 0) return nullptr;
        Parcel* top = heap[0];
        heap[0] = heap[heapSize - 1];
        heapSize--;
        heapifyDown(0);
        return top;
    }

    Parcel* getAtIndex(int i) { return (i < heapSize) ? heap[i] : nullptr; }

    bool isEmpty() { return heapSize == 0; }
    int size() { return heapSize; }
};

// --- 4.6 HASH TABLE (FOR TRACKING) ---
class TrackingHashTable {
private:
    ParcelList* buckets[HASH_TABLE_SIZE];

    int hashFunc(int id) {
        return id % HASH_TABLE_SIZE;
    }

public:
    TrackingHashTable() {
        for (int i = 0; i < HASH_TABLE_SIZE; i++) {
            buckets[i] = new ParcelList();
        }
    }

    void insert(Parcel* p) {
        int idx = hashFunc(p->id);
        buckets[idx]->pushBack(p);
    }

    Parcel* search(int id) {
        int idx = hashFunc(id);
        ParcelNode* curr = buckets[idx]->head;
        while (curr) {
            if (curr->data->id == id) return curr->data;
            curr = curr->next;
        }
        return nullptr;
    }
};

// --- 4.7 BINARY SEARCH TREE ---
struct ParcelTreeNode {
    Parcel* data;
    ParcelTreeNode* left;
    ParcelTreeNode* right;
    ParcelTreeNode(Parcel* val) : data(val), left(nullptr), right(nullptr) {}
};

class ArchiveBST {
private:
    ParcelTreeNode* root;

    void insertRec(ParcelTreeNode*& node, Parcel* p) {
        if (!node) {
            node = new ParcelTreeNode(p);
            return;
        }
        if (p->id < node->data->id) insertRec(node->left, p);
        else if (p->id > node->data->id) insertRec(node->right, p);
    }

    void inOrderRec(ParcelTreeNode* node) {
        if (!node) return;
        inOrderRec(node->left);
        node->data->displayTableRow();
        inOrderRec(node->right);
    }

public:
    ArchiveBST() : root(nullptr) {}
    void insert(Parcel* p) { insertRec(root, p); }
    void displayInOrder() { 
        if (!root) { cout << YELLOW << "    (History Archive is Empty)" << RESET << endl; return; }
        cout << BLUE << " | " << setw(5) << "ID" 
             << " | " << setw(12) << "SOURCE" 
             << " | " << setw(12) << "DEST"
             << " | " << setw(6) << "WGT"
             << " | " << setw(5) << "PRI"
             << " | " << setw(25) << "FINAL STATUS"
             << " | " << setw(8) << "RIDER"
             << " | " << RESET << endl;
        UIHelper::printLine();
        inOrderRec(root); 
    }
};

// ==========================================
// 5. GRAPH MODULE (ROUTING)
// ==========================================

struct Edge {
    int destCityID;
    int weight;       
    int baseDistance; 
    bool isBlocked;   
    bool hasTraffic;

    Edge(int d, int w) : destCityID(d), weight(w), baseDistance(w), isBlocked(false), hasTraffic(false) {}
};

struct EdgeNode {
    Edge data;
    EdgeNode* next;
    EdgeNode(Edge val) : data(val), next(nullptr) {}
};

class EdgeList {
public:
    EdgeNode* head;
    EdgeList() : head(nullptr) {}
    
    void pushBack(Edge val) {
        EdgeNode* n = new EdgeNode(val);
        if(!head) head = n;
        else {
            EdgeNode* temp = head;
            while(temp->next) temp = temp->next;
            temp->next = n;
        }
    }
};

struct PathInfo {
    int totalDist;
    string pathDescription;
    bool isValid;
    bool isBlocked;
    bool containsTraffic;
};

// String Stack
struct StringNode {
    string data;
    StringNode* next;
    StringNode(string val) : data(val), next(nullptr) {}
};

class StringStack {
private:
    StringNode* top;
public:
    StringStack() : top(nullptr) {}
    void push(string val) {
        StringNode* n = new StringNode(val);
        n->next = top;
        top = n;
    }
    string pop() {
        if(!top) return "";
        StringNode* temp = top;
        string val = temp->data;
        top = top->next;
        delete temp;
        return val;
    }
    bool isEmpty() { return top == nullptr; }
};

class LogisticsGraph {
private:
    struct CityNode {
        int id;
        string name;
        EdgeList edges;
    };
    
    CityNode cities[MAX_CITIES];
    int numCities;

public:
    LogisticsGraph() : numCities(0) {}

    void addCity(int id, string name) {
        if (id >= MAX_CITIES) return;
        cities[id].id = id;
        cities[id].name = name;
        if (id >= numCities) numCities = id + 1;
    }

    void addRoad(int u, int v, int dist) {
        if (u < numCities && v < numCities) {
            cities[u].edges.pushBack(Edge(v, dist));
            cities[v].edges.pushBack(Edge(u, dist)); 
        }
    }

    string getCityName(int id) {
        if (id >= 0 && id < numCities && cities[id].name != "") return cities[id].name;
        return "Unknown";
    }

    int getCityID(string name) {
        for (int i = 0; i < numCities; i++) {
            if (cities[i].name == name) return i;
        }
        return -1;
    }

    PathInfo calculateShortestPath(int start, int end, int avoidEdgeU = -1, int avoidEdgeV = -1) {
        int dist[MAX_CITIES];
        int parent[MAX_CITIES];
        bool visited[MAX_CITIES];

        for (int i = 0; i < MAX_CITIES; i++) {
            dist[i] = INT_MAX;
            visited[i] = false;
            parent[i] = -1;
        }

        dist[start] = 0;

        for (int count = 0; count < numCities; count++) {
            int u = -1, minVal = INT_MAX;
            for (int i = 0; i < numCities; i++) {
                if (cities[i].name != "" && !visited[i] && dist[i] < minVal) {
                    minVal = dist[i];
                    u = i;
                }
            }

            if (u == -1 || dist[u] == INT_MAX) break; 
            visited[u] = true;

            EdgeNode* curr = cities[u].edges.head;
            while (curr) {
                int v = curr->data.destCityID;
                bool isRestrictedEdge = (u == avoidEdgeU && v == avoidEdgeV) || (u == avoidEdgeV && v == avoidEdgeU);
                
                if (!isRestrictedEdge) {
                    int weight = curr->data.weight;
                    if (!visited[v] && dist[u] + weight < dist[v]) {
                        dist[v] = dist[u] + weight;
                        parent[v] = u;
                    }
                }
                curr = curr->next;
            }
        }

        PathInfo result;
        result.totalDist = dist[end];
        result.isValid = (dist[end] != INT_MAX);
        result.isBlocked = false;
        result.containsTraffic = false;
        
        if (result.isValid) {
            string pathStr = "";
            int curr = end;
            StringStack pathStack;
            
            while (curr != -1) {
                pathStack.push(cities[curr].name);
                int prev = parent[curr];
                if (prev != -1) {
                    EdgeNode* edgeNode = cities[prev].edges.head;
                    while(edgeNode) {
                        if (edgeNode->data.destCityID == curr) {
                            if (edgeNode->data.hasTraffic) result.containsTraffic = true;
                            if (edgeNode->data.isBlocked) result.isBlocked = true;
                            break;
                        }
                        edgeNode = edgeNode->next;
                    }
                }
                curr = prev;
            }

            while(!pathStack.isEmpty()) {
                pathStr += pathStack.pop();
                if (!pathStack.isEmpty()) pathStr += " -> ";
            }
            result.pathDescription = pathStr;
            if (result.totalDist >= 999999) result.isBlocked = true; 
        } else {
            result.pathDescription = "No Path Available";
        }

        return result;
    }

    PathInfo calculateAlternativeRoute(int start, int end) {
        PathInfo best = calculateShortestPath(start, end);
        if (!best.isValid) return best; 
        
        EdgeNode* curr = cities[start].edges.head;
        PathInfo secondBest;
        secondBest.totalDist = INT_MAX;
        secondBest.isValid = false;
        secondBest.isBlocked = false;

        while(curr) {
            PathInfo candidate = calculateShortestPath(start, end, start, curr->data.destCityID);
            if (candidate.isValid && candidate.totalDist >= best.totalDist && candidate.totalDist < secondBest.totalDist) {
                 if(candidate.pathDescription != best.pathDescription)
                    secondBest = candidate;
            }
            curr = curr->next;
        }
        
        if (secondBest.totalDist == INT_MAX) {
            secondBest.isValid = false;
            secondBest.pathDescription = "No distinct alternative route found.";
        }
        return secondBest;
    }

    bool setRoadStatus(int u, int v, int status) {
        bool found = false;
        
        EdgeNode* e1 = cities[u].edges.head;
        while(e1) {
            if (e1->data.destCityID == v) {
                found = true;
                if (status == 1) { 
                    e1->data.isBlocked = false;
                    e1->data.hasTraffic = false;
                    e1->data.weight = e1->data.baseDistance;
                } else if (status == 2) { 
                    e1->data.isBlocked = false;
                    e1->data.hasTraffic = true;
                    e1->data.weight = e1->data.baseDistance * 3; 
                } else if (status == 3) { 
                    e1->data.isBlocked = true;
                    e1->data.hasTraffic = false; 
                    e1->data.weight = e1->data.baseDistance; 
                }
                break;
            }
            e1 = e1->next;
        }

        EdgeNode* e2 = cities[v].edges.head;
        while(e2) {
            if (e2->data.destCityID == u) {
                found = true;
                if (status == 1) { 
                    e2->data.isBlocked = false;
                    e2->data.hasTraffic = false;
                    e2->data.weight = e2->data.baseDistance;
                } else if (status == 2) { 
                    e2->data.isBlocked = false;
                    e2->data.hasTraffic = true;
                    e2->data.weight = e2->data.baseDistance * 3; 
                } else if (status == 3) { 
                    e2->data.isBlocked = true;
                    e2->data.hasTraffic = false; 
                    e2->data.weight = e2->data.baseDistance;
                }
                break;
            }
            e2 = e2->next;
        }
        return found;
    }
    
    void printGraphTable() {
        UIHelper::printHeader("ROAD NETWORK STATUS REPORT");
        cout << BLUE << " | " << setw(15) << "CITY A" 
             << " | " << setw(15) << "CITY B" 
             << " | " << setw(8) << "DIST(km)"
             << " | " << setw(16) << "STATUS" 
             << " |" << RESET << endl;
        UIHelper::printLine();

        for (int i=0; i<numCities; i++) {
            if(cities[i].name == "") continue; // Skip empty/unused slots
            EdgeNode* curr = cities[i].edges.head;
            while(curr) {
                if(curr->data.destCityID > i) {
                    string stat = "Normal";
                    string color = GREEN;
                    if(curr->data.isBlocked) { stat = "BLOCKED"; color = RED; }
                    else if(curr->data.hasTraffic) { stat = "HEAVY TRAFFIC"; color = YELLOW; }

                    cout << " | " << setw(15) << cities[i].name 
                         << " | " << setw(15) << cities[curr->data.destCityID].name
                         << " | " << setw(8) << curr->data.baseDistance
                         << " | " << color << setw(16) << stat << RESET 
                         << " |" << endl;
                }
                curr = curr->next;
            }
        }
        UIHelper::printLine();
        
        cout << BOLD << "\n >> CITY ID REFERENCE (Use these IDs for input):" << RESET << endl;
        for(int i=0; i<numCities; i+=2) {
             if(cities[i].name == "") { i--; continue; } 
             cout << left << setw(4) << i << ": " << setw(15) << cities[i].name;
             if(i+1 < numCities && cities[i+1].name != "") cout << setw(4) << i+1 << ": " << setw(15) << cities[i+1].name;
             cout << endl;
        }
        UIHelper::printLine();
    }
    
    int getCityCount() { return numCities; }
};

// ==========================================
// 6. MAIN CONTROLLER CLASS
// ==========================================

class SwiftExEngine {
private:
    TrackingHashTable trackingSystem;
    LogisticsGraph routingEngine;
    ParcelPriorityQueue warehouseQueue; 
    ArchiveBST archive;                 
    
    ParcelQueue pickupQueue;   
    ParcelList transitList;    
    
    Rider* fleet[5];
    int fleetSize;

    UndoStack undoStack;
    ParcelList masterList;

public:
    SwiftExEngine() {
        initMap();
        initFleet();
    }

    void initMap() {
        // Start from ID 1
        routingEngine.addCity(1, "Lahore");
        routingEngine.addCity(2, "Islamabad");
        routingEngine.addCity(3, "Karachi");
        routingEngine.addCity(4, "Rawalpindi");
        routingEngine.addCity(5, "Faisalabad");
        routingEngine.addCity(6, "Multan");
        routingEngine.addCity(7, "Peshawar");
        routingEngine.addCity(8, "Quetta");
        routingEngine.addCity(9, "Sialkot");
        routingEngine.addCity(10, "Gujranwala");

        routingEngine.addRoad(1, 2, 375); // LHR-ISL
        routingEngine.addRoad(2, 4, 20);  // ISL-RWP
        routingEngine.addRoad(1, 10, 70); // LHR-GUJ
        routingEngine.addRoad(10, 9, 55); // GUJ-SKT
        routingEngine.addRoad(1, 5, 180); // LHR-FSD
        routingEngine.addRoad(5, 6, 250); // FSD-MUX
        routingEngine.addRoad(6, 3, 900); // MUX-KHI
        routingEngine.addRoad(6, 8, 650); // MUX-QTA
        routingEngine.addRoad(3, 8, 690); // KHI-QTA
        routingEngine.addRoad(2, 7, 190); // ISL-PEW
    }

    void initFleet() {
        // Updated Capacities: Bike=50, Van=200, Truck=1000
        // REORDERED FOR PRIORITY: Bikes first, then Vans, then Truck
        // This ensures the greedy dispatch logic tries Bikes before larger vehicles.
        fleet[0] = new Rider(101, "Ahmed (Bike)", "Bike", 50.0);
        fleet[1] = new Rider(105, "Ehsan (Bike)", "Bike", 50.0);
        fleet[2] = new Rider(102, "Bilal (Van)", "Van", 200.0);
        fleet[3] = new Rider(104, "Dawood (Van)", "Van", 200.0);
        fleet[4] = new Rider(103, "Chacha (Truck)", "Truck", 1000.0);
        fleetSize = 5;
    }

    void registerParcel() {
        UIHelper::printHeader("REGISTER NEW PARCEL");
        routingEngine.printGraphTable();

        int id;
        while(true) {
            id = UIHelper::getIntInput(" >> Enter Parcel ID (Positive, 0 to Cancel): ", 0, 99999);
            if (id == 0) return;
            if (trackingSystem.search(id) == nullptr) break;
            cout << RED << " [!] Error: Parcel ID " << id << " already exists. Try another." << RESET << endl;
        }

        int srcID;
        while(true) {
            srcID = UIHelper::getIntInput(" >> Enter Source City ID (0 to Cancel): ", 0, 100);
            if (srcID == 0) return;
            if (routingEngine.getCityName(srcID) != "Unknown") break;
            cout << RED << " [!] Invalid Source City ID. Try again." << RESET << endl;
        }
        
        int destID;
        while(true) {
            destID = UIHelper::getIntInput(" >> Enter Destination City ID (0 to Cancel): ", 0, 100);
            if (destID == 0) return;
            if (routingEngine.getCityName(destID) != "Unknown") break;
            cout << RED << " [!] Invalid Destination City ID. Try again." << RESET << endl;
        }
        
        if (srcID == destID) {
            cout << RED << " [!] Error: Source and Destination cannot be the same." << RESET << endl;
            UIHelper::pressEnterToContinue();
            return;
        }

        string src = routingEngine.getCityName(srcID);
        string dest = routingEngine.getCityName(destID);

        double w;
        w = UIHelper::getDoubleInput(" >> Enter Weight (kg) (0 to Cancel): ", 0.0, 1000.0);
        if (w == 0.0) return;

        cout << " [1] Overnight (High Priority)\n [2] 2-Day (Medium Priority)\n [3] Standard (Low Priority)" << endl;
        int p = UIHelper::getIntInput(" >> Select Priority Level: ", 1, 3);

        Parcel* newP = new Parcel(id, src, dest, w, p);
        
        cout << "\n" << BOLD << " >> CALCULATING ROUTES..." << RESET << endl;
        PathInfo best = routingEngine.calculateShortestPath(srcID, destID);
        PathInfo alt = routingEngine.calculateAlternativeRoute(srcID, destID);

        if (!best.isValid) {
            cout << RED << " [!] CRITICAL: No path exists between these cities (Network Disconnected)." << RESET << endl;
            delete newP;
            UIHelper::pressEnterToContinue();
            return;
        }

        cout << CYAN << " [1] RECOMMENDED: " << RESET << best.pathDescription 
             << (best.isBlocked ? (RED + " [BLOCKED]" + RESET) : (GREEN + " [NORMAL]" + RESET)) 
             << " (" << best.totalDist << " km)" << endl;
             
        if (alt.isValid) {
            cout << CYAN << " [2] ALTERNATIVE: " << RESET << alt.pathDescription 
                 << (alt.isBlocked ? (RED + " [BLOCKED]" + RESET) : (GREEN + " [NORMAL]" + RESET)) 
                 << " (" << alt.totalDist << " km)" << endl;
        } else {
             cout << YELLOW << " [2] No distinct alternative route available." << RESET << endl;
        }

        PathInfo selected = best;
        if (best.isBlocked || (alt.isValid && alt.isBlocked)) {
             cout << YELLOW << "\n >> ALERT: One or more routes have blocks." << RESET << endl;
             int choice = UIHelper::getIntInput(" >> Force select route [1] or [2]? ", 1, 2);
             if (choice == 2 && alt.isValid) selected = alt;
             else selected = best;
        } else {
             cout << GREEN << "\n >> Auto-assigning Recommended Route (Optimal)." << RESET << endl;
        }

        if (selected.containsTraffic) {
            cout << MAGENTA << " [ALERT] Selected route contains TRAFFIC. Delays expected." << RESET << endl;
        }
        
        if (selected.isBlocked) {
             cout << RED << " [WARNING] You have selected a BLOCKED route. Delivery may fail." << RESET << endl;
             newP->willFailOnPath = true;
        }

        newP->assignedRoute = selected.pathDescription;
        newP->totalDistanceKm = selected.totalDist;
        newP->estimatedDurationSec = selected.totalDist / SIM_SPEED_KM_PER_SEC;
        
        // --- COST CALCULATION BREAKDOWN ---
        double baseFee = 100.0;
        double weightFee = newP->weight * 15.0;
        double priorityFee = (newP->priorityLevel == 1) ? 500.0 : ((newP->priorityLevel == 2) ? 200.0 : 0.0);
        double distanceFee = newP->totalDistanceKm * 5.0; // UPDATED DISTANCE RATE
        
        newP->shippingCost = baseFee + weightFee + priorityFee + distanceFee;
        
        cout << endl;
        UIHelper::printSubHeader("COST BREAKDOWN");
        cout << " + Base Fee:       " << setw(8) << fixed << setprecision(2) << baseFee << " PKR" << endl;
        cout << " + Weight Charge:  " << setw(8) << weightFee << " PKR (" << w << " kg * 15)" << endl;
        cout << " + Priority Fee:   " << setw(8) << priorityFee << " PKR (" << newP->getPriorityStr() << ")" << endl;
        cout << " + Distance Fee:   " << setw(8) << distanceFee << " PKR (" << newP->totalDistanceKm << " km * 5.0)" << endl;
        UIHelper::printLine();
        cout << BOLD << " = TOTAL COST:     " << GREEN << setw(8) << newP->shippingCost << " PKR" << RESET << endl;
        UIHelper::printLine();
        
        newP->addToHistory("Route Assigned: " + selected.pathDescription);

        masterList.pushBack(newP);
        trackingSystem.insert(newP);
        pickupQueue.enqueue(newP); 

        cout << GREEN << " >> Success: Parcel Registered and placed in Pickup Queue." << RESET << endl;
        cout << " >> Estimated Duration: " << newP->estimatedDurationSec << " seconds" << endl;
        UIHelper::pressEnterToContinue();
    }

    void processPickupQueue() {
        UIHelper::printHeader("PROCESS PICKUP QUEUE");
        if (pickupQueue.isEmpty()) {
            cout << YELLOW << " >> Pickup Queue is empty. No parcels to process." << RESET << endl;
            UIHelper::pressEnterToContinue();
            return;
        }

        while (!pickupQueue.isEmpty()) {
            Parcel* p = pickupQueue.dequeue();
            p->status = p->sourceCity + " Warehouse";
            p->addToHistory("Processed from Pickup Queue. Moved to " + p->sourceCity + " Warehouse Sorting.");
            warehouseQueue.insert(p); 
            cout << " >> Processed ID #" << p->id << " (" << p->getPriorityStr() << ") -> Moved to Warehouse." << endl;
        }
        cout << GREEN << " >> All items moved to Warehouse Heap." << RESET << endl;
        UIHelper::pressEnterToContinue();
    }

    void dispatchFromWarehouse() {
        UIHelper::printHeader("WAREHOUSE DISPATCH (RIDER ASSIGNMENT)");
        
        if (warehouseQueue.isEmpty()) {
            cout << YELLOW << " >> Warehouse is empty. Nothing to dispatch." << RESET << endl;
            // CHECK IF ITEMS ARE STUCK IN PICKUP
            if(!pickupQueue.isEmpty()) {
                cout << RED << " [!] ALERT: " << pickupQueue.count() << " parcels are waiting in Pickup Queue. Please run 'Process Pickup Queue' first." << RESET << endl;
            }
            UIHelper::pressEnterToContinue();
            return;
        }

        cout << " >> Available Fleet:" << endl;
        cout << BLUE << " | " << setw(4) << "ID" << " | " << setw(15) << "Name" << " | " << setw(10) << "Capacity" << " |" << RESET << endl;
        UIHelper::printLine();
        for(int i=0; i<fleetSize; i++) {
            cout << " | " << setw(4) << fleet[i]->id 
                 << " | " << setw(15) << fleet[i]->name 
                 << " | " << setw(10) << (to_string((int)fleet[i]->maxLoadCapacity) + "kg") << " |" << endl;
        }
        UIHelper::printLine();

        int dispatchedCount = 0;
        ParcelStack tempStack;

        // Visual confirmation of Priority Processing
        cout << BOLD << " >> Processing Parcels by Priority (High -> Med -> Low)..." << RESET << endl;

        while(!warehouseQueue.isEmpty()) {
            Parcel* p = warehouseQueue.extractMin(); 
            bool assigned = false;

            // Strategy: Priority for Empty Riders to balance load ("Assign to another rider")
            
            // 1. Try to find an IDLE rider first
            for(int i=0; i<fleetSize; i++) {
                if (fleet[i]->status == "Idle" && fleet[i]->canCarry(p->weight)) {
                    fleet[i]->assignParcel(p->weight);
                    
                    p->status = "In Transit";
                    p->dispatchTime = time(0);
                    p->lastUpdateTime = time(0);
                    p->assignedRiderName = fleet[i]->name;
                    p->addToHistory("Dispatched: Assigned to " + fleet[i]->name);
                    
                    transitList.pushBack(p);
                    ActionLog log = {"DISPATCH", p, fleet[i]};
                    undoStack.push(log);
                    
                    cout << GREEN << " >> [PRIORITY: " << p->getPriorityStr() << "] Parcel #" << p->id << " assigned to " << fleet[i]->name << " (New Assignment)" << RESET << endl;
                    assigned = true;
                    dispatchedCount++;
                    break;
                }
            }

            // 2. If no idle rider found, try to fit in a busy rider (Capacity Optimization)
            if (!assigned) {
                for(int i=0; i<fleetSize; i++) {
                    if (fleet[i]->canCarry(p->weight)) {
                        fleet[i]->assignParcel(p->weight);
                        
                        p->status = "In Transit";
                        p->dispatchTime = time(0);
                        p->lastUpdateTime = time(0);
                        p->assignedRiderName = fleet[i]->name;
                        p->addToHistory("Dispatched: Assigned to " + fleet[i]->name);
                        
                        transitList.pushBack(p);
                        ActionLog log = {"DISPATCH", p, fleet[i]};
                        undoStack.push(log);
                        
                        cout << YELLOW << " >> [PRIORITY: " << p->getPriorityStr() << "] Parcel #" << p->id << " added to " << fleet[i]->name << " (Load Optimization)" << RESET << endl;
                        assigned = true;
                        dispatchedCount++;
                        break;
                    }
                }
            }

            if (!assigned) {
                cout << RED << " >> [PRIORITY: " << p->getPriorityStr() << "] Parcel #" << p->id << " (" << p->weight << "kg) - NO RIDER CAPACITY. Returning to Storage." << RESET << endl;
                tempStack.push(p);
            }
        }

        while(!tempStack.isEmpty()) {
            warehouseQueue.insert(tempStack.pop());
        }

        cout << endl << CYAN << " >> Dispatch Complete. Total Dispatched: " << dispatchedCount << RESET << endl;
        UIHelper::pressEnterToContinue();
    }

    void undoLastOp() {
        UIHelper::printHeader("UNDO OPERATIONS LOG");
        if (undoStack.isEmpty()) {
            cout << RED << " >> No operations to undo." << RESET << endl;
            UIHelper::pressEnterToContinue();
            return;
        }

        ActionLog last = undoStack.pop();
        if (last.actionType == "DISPATCH") {
            Parcel* p = last.parcelPtr;
            Rider* r = last.riderPtr;
            
            if (transitList.remove(p)) {
                p->status = p->sourceCity + " Warehouse";
                p->assignedRiderName = "None";
                p->addToHistory("UNDO: Dispatch reversed. Returned to Warehouse.");
                p->dispatchTime = 0;
                
                r->currentLoad -= p->weight;
                if (r->currentLoad == 0) r->status = "Idle";
                
                warehouseQueue.insert(p);
                cout << YELLOW << " >> UNDO SUCCESS: Parcel #" << p->id << " removed from " << r->name << " and returned to Warehouse." << RESET << endl;
            } else {
                cout << RED << " >> UNDO FAILED: Parcel not found in transit (maybe already delivered?)" << RESET << endl;
            }
        }
        UIHelper::pressEnterToContinue();
    }

    void updateSimulation() {
        time_t now = time(0);
        ParcelNode* curr = transitList.head;
        ParcelList toRemove; 

        while (curr) {
            Parcel* p = curr->data;
            double secondsElapsed = difftime(now, p->dispatchTime);
            double timeSinceLastUpdate = difftime(now, p->lastUpdateTime);

            // 1. MISSING LOGIC (Inactive/Stagnant for 300s)
            // If status hasn't changed for 300s, declare MISSING and REMOVE from active flow
            if (timeSinceLastUpdate > MISSING_PARCEL_THRESHOLD && p->status == "In Transit") {
                p->status = "MISSING";
                p->addToHistory("ALERT: Parcel declared MISSING due to inactivity.");
                cout << RED << " >> ALERT: Parcel #" << p->id << " status hasn't changed for 300s. Declared MISSING." << RESET << endl;
                
                // Remove from transit list (active flow). 
                // It stays in masterList for the "Missing Report".
                toRemove.pushBack(p); 
                curr = curr->next;
                continue; 
            }

            // 2. FAILED DELIVERY LOGIC (Blocked Road)
            else if (p->willFailOnPath && !p->isReturning && p->status == "In Transit") {
                // Simulate reaching the block point or destination time
                if (secondsElapsed > (p->estimatedDurationSec * 0.2)) {
                    p->status = p->sourceCity + " Warehouse"; // Reset to source
                    p->addToHistory("FAILURE: Route Blocked. Returned to Source Warehouse.");
                    cout << RED << " >> Delivery Failed for Parcel #" << p->id << " due to blockage. Returned to " << p->sourceCity << " Warehouse." << RESET << endl;
                    
                    // Return to Warehouse (System retains it)
                    warehouseQueue.insert(p);
                    toRemove.pushBack(p); // Remove from transit list only
                }
            }

            // 3. SUCCESSFUL DELIVERY
            else if (secondsElapsed >= p->estimatedDurationSec) {
                 if (p->status == "Returning") {
                    p->status = "Returned to Sender";
                    p->addToHistory("Process Complete: Item returned to sender.");
                 } else {
                    p->status = "Delivered";
                    p->addToHistory("Process Complete: Successfully Delivered.");
                 }
                 
                 p->addToHistory("Archived: Moved to Historical Record.");
                 archive.insert(p);
                 toRemove.pushBack(p);
            }
            
            curr = curr->next;
        }

        // Cleanup transit list
        while (!toRemove.isEmpty()) {
            transitList.remove(toRemove.popFront());
        }
    }

    void trackParcel() {
        UIHelper::printHeader("TRACK PARCEL");
        int id = UIHelper::getIntInput(" >> Enter Parcel ID (0 to Cancel): ", 0, 99999);
        if (id == 0) return;
        
        Parcel* p = trackingSystem.search(id);
        
        if (p) {
            p->displayFullDetails();
        } else {
            cout << RED << " >> Error: Parcel ID not found in the system." << RESET << endl;
        }
        UIHelper::pressEnterToContinue();
    }

    void viewAllParcels(string filter = "ALL") {
        UIHelper::printHeader("SHIPMENT LIST (" + filter + ")");
        if (masterList.isEmpty()) {
            cout << YELLOW << " >> No parcels in the system." << RESET << endl;
            UIHelper::pressEnterToContinue();
            return;
        }
        
        cout << BLUE << " | " << setw(5) << "ID" 
             << " | " << setw(12) << "SOURCE" 
             << " | " << setw(12) << "DEST"
             << " | " << setw(6) << "WGT"
             << " | " << setw(5) << "PRI"
             << " | " << setw(25) << "STATUS"
             << " | " << setw(8) << "RIDER"
             << " | " << RESET << endl;
        UIHelper::printLine();
        
        ParcelNode* curr = masterList.head;
        int count = 0;
        while(curr) {
            bool show = false;
            if (filter == "ALL") show = true;
            else if (filter == "TRANSIT" && curr->data->status == "In Transit") show = true;
            else if (filter == "WAREHOUSE" && curr->data->status.find("Warehouse") != string::npos) show = true;
            
            if (show) {
                curr->data->displayTableRow();
                count++;
            }
            curr = curr->next;
        }
        if (count == 0) cout << YELLOW << "    No parcels found matching this filter." << RESET << endl;
        UIHelper::printLine();
        UIHelper::pressEnterToContinue();
    }

    void viewHighPriorityQueue() {
        UIHelper::printHeader("HIGH PRIORITY QUEUE VIEW");
        if (warehouseQueue.isEmpty()) {
             cout << YELLOW << " >> Warehouse is empty." << RESET << endl;
        } else {
             cout << " (Showing items currently in Priority Queue)" << endl;
             cout << BLUE << " | " << setw(5) << "ID" << " | " << setw(5) << "PRI" << " | " << setw(6) << "WGT" << " |" << RESET << endl;
             UIHelper::printLine();
             
             for(int i=0; i<warehouseQueue.size() && i<10; i++) {
                  Parcel* p = warehouseQueue.getAtIndex(i); 
                  if(p) {
                      cout << " | " << setw(5) << p->id << " | " << setw(5) << p->getPriorityStr() << " | " << setw(6) << p->weight << " |" << endl;
                  }
             }
        }
        UIHelper::pressEnterToContinue();
    }

    void viewFleetStatus() {
        UIHelper::printHeader("FLEET STATUS REPORT");
        cout << BLUE << " | " << setw(4) << "ID" 
             << " | " << setw(15) << "NAME" 
             << " | " << setw(8) << "TYPE"
             << " | " << setw(8) << "CAP(kg)"
             << " | " << setw(8) << "LOAD"
             << " | " << setw(10) << "STATUS"
             << " | " << setw(6) << "UTIL%" 
             << " |" << RESET << endl;
        UIHelper::printLine();
        
        for(int i=0; i<fleetSize; i++) {
            fleet[i]->displayRow();
        }
        UIHelper::printLine();
        UIHelper::pressEnterToContinue();
    }

    void viewAnalytics() {
        UIHelper::printHeader("SYSTEM ANALYTICS REPORT");
        int total = masterList.size;
        int delivered = 0;
        int failed = 0;
        int transit = 0;
        double revenue = 0.0;

        ParcelNode* curr = masterList.head;
        while(curr) {
            if(curr->data->status == "Delivered") { delivered++; revenue += curr->data->shippingCost; }
            else if(curr->data->status == "Returned to Sender" || curr->data->status == "Delivery Failed") failed++;
            else if(curr->data->status == "In Transit") transit++;
            curr = curr->next;
        }

        cout << " Total Parcels Registered: " << BOLD << total << RESET << endl;
        cout << " Successfully Delivered:   " << GREEN << delivered << RESET << endl;
        cout << " Failed / Returned:        " << RED << failed << RESET << endl;
        cout << " Currently In Transit:     " << BLUE << transit << RESET << endl;
        cout << " Total Revenue Generated:  " << YELLOW << "PKR " << fixed << setprecision(2) << revenue << RESET << endl;
        
        UIHelper::pressEnterToContinue();
    }
    
    void resetSystem() {
        UIHelper::printHeader("RESET SYSTEM SIMULATION");
        cout << RED << " Warning: This will clear all current transit data. (Parcels remain in record)" << RESET << endl;
        int confirm = UIHelper::getIntInput(" Enter 1 to Confirm Reset (0 to Cancel): ", 0, 1);
        if(confirm == 1) {
            for(int i=0; i<fleetSize; i++) fleet[i]->reset();
            cout << GREEN << " >> Riders returned to base. Day reset." << RESET << endl;
        }
        UIHelper::pressEnterToContinue();
    }
    
    void adminControls() {
        while(true) {
            updateSimulation();
            UIHelper::clearScreen();
            UIHelper::printHeader("ADMINISTRATION PANEL");
            UIHelper::printMenuOption(1, "View Network Map (Road Status)");
            UIHelper::printMenuOption(2, "Update Road Status (Traffic/Block)");
            UIHelper::printMenuOption(3, "View Missing/Stuck Parcels Report");
            UIHelper::printMenuOption(4, "View Delivered History (Archive)");
            UIHelper::printMenuOption(5, "View Detailed Fleet Utilization");
            UIHelper::printMenuOption(6, "View Master Shipment List (All Parcels)");
            UIHelper::printMenuOption(7, "View System Analytics & Revenue");
            UIHelper::printMenuOption(8, "Reset Daily Simulation (End Day)");
            UIHelper::printMenuOption(0, "Log Out");
            UIHelper::printLine();
            
            int choice = UIHelper::getIntInput(" >> Select Option: ", 0, 8);
            
            if (choice == 0) break;
            
            switch(choice) {
                case 1: 
                    routingEngine.printGraphTable(); 
                    UIHelper::pressEnterToContinue();
                    break;
                case 2: {
                    routingEngine.printGraphTable();
                    int u = UIHelper::getIntInput(" >> Source City ID (0 to Cancel): ", 0, 100);
                    if (u==0) break;
                    int v = UIHelper::getIntInput(" >> Dest City ID (0 to Cancel):   ", 0, 100);
                    if (v==0) break;

                    if(routingEngine.getCityName(u) != "Unknown" && routingEngine.getCityName(v) != "Unknown") {
                        cout << " [1] Normal\n [2] Heavy Traffic\n [3] Blocked" << endl;
                        int s = UIHelper::getIntInput(" >> New Status: ", 1, 3);
                        bool updated = routingEngine.setRoadStatus(u, v, s);
                        
                        if (updated) {
                            cout << GREEN << " >> Road Status Updated Successfully." << RESET << endl;
                            ParcelNode* curr = transitList.head;
                            while(curr) {
                                Parcel* p = curr->data;
                                int pu = routingEngine.getCityID(p->sourceCity);
                                int pv = routingEngine.getCityID(p->destCity);
                                PathInfo check = routingEngine.calculateShortestPath(pu, pv);
                                if (!check.isValid) {
                                    p->willFailOnPath = true;
                                    cout << RED << " >> ALERT: Parcel #" << p->id << " is now on a blocked path!" << RESET << endl;
                                }
                                curr = curr->next;
                            }
                        } else {
                            cout << RED << " [!] Error: No direct road exists between these two cities." << RESET << endl;
                        }
                    } else {
                        cout << RED << " [!] Invalid City IDs." << RESET << endl;
                    }
                    UIHelper::pressEnterToContinue();
                    break;
                }
                case 3: {
                    UIHelper::printSubHeader("MISSING PARCEL REPORT");
                    ParcelNode* curr = masterList.head;
                    bool found = false;
                    while(curr) {
                        if (curr->data->isMissing()) {
                            curr->data->displayTableRow();
                            found = true;
                        }
                        curr = curr->next;
                    }
                    if (!found) cout << GREEN << " >> No missing parcels detected." << RESET << endl;
                    UIHelper::pressEnterToContinue();
                    break;
                }
                case 4: 
                    UIHelper::printSubHeader("DELIVERED HISTORY (ARCHIVE)");
                    archive.displayInOrder();
                    UIHelper::pressEnterToContinue();
                    break;
                case 5: viewFleetStatus(); break;
                case 6: viewAllParcels(); break;
                case 7: viewAnalytics(); break;
                case 8: resetSystem(); break;
            }
        }
    }

    void staffMenu() {
        while(true) {
            updateSimulation(); 
            UIHelper::clearScreen();
            UIHelper::printHeader("STAFF DASHBOARD");
            UIHelper::printMenuOption(1, "Register New Parcel");
            UIHelper::printMenuOption(2, "Process Pickup Queue (-> Warehouse)");
            UIHelper::printMenuOption(3, "Dispatch Warehouse (-> Riders)");
            UIHelper::printMenuOption(4, "Track Parcel Details");
            UIHelper::printMenuOption(5, "Undo Last Dispatch Operation");
            UIHelper::printMenuOption(6, "View Queue Status (Pending/Warehouse)");
            UIHelper::printMenuOption(7, "View Active Shipments (In Transit)");
            UIHelper::printMenuOption(8, "View High Priority Parcels (Preview)");
            UIHelper::printMenuOption(0, "Log Out");
            UIHelper::printLine();
            
            int choice = UIHelper::getIntInput(" >> Select Option: ", 0, 8);
            
            switch(choice) {
                case 1: registerParcel(); break;
                case 2: processPickupQueue(); break;
                case 3: dispatchFromWarehouse(); break;
                case 4: trackParcel(); break;
                case 5: undoLastOp(); break;
                case 6: {
                    UIHelper::printSubHeader("1. PENDING (PICKUP QUEUE)");
                    if (pickupQueue.isEmpty()) {
                        cout << YELLOW << " No parcels waiting for pickup." << RESET << endl;
                    } else {
                        cout << BLUE << " | " << setw(5) << "ID" << " | " << setw(12) << "SOURCE" << " | " << setw(12) << "DEST" << " |" << RESET << endl;
                        ParcelNode* curr = pickupQueue.getHead();
                        while(curr) {
                            cout << " | " << setw(5) << curr->data->id 
                                 << " | " << setw(12) << curr->data->sourceCity.substr(0,12) 
                                 << " | " << setw(12) << curr->data->destCity.substr(0,12) << " |" << endl;
                            curr = curr->next;
                        }
                    }
                    cout << "\n ----------------------------- \n";
                    UIHelper::printSubHeader("2. WAREHOUSE SORTING QUEUE");
                    cout << " Warehouse Heap Count:  " << warehouseQueue.size() << endl;
                    viewAllParcels("WAREHOUSE");
                    break;
                }
                case 7: {
                    viewAllParcels("TRANSIT");
                    break;
                }
                case 8: viewHighPriorityQueue(); break;
                case 0: return;
            }
        }
    }

    void login() {
        while(true) {
            UIHelper::clearScreen();
            UIHelper::printBanner();
            // UPDATED: Manually colored menu options as requested
            cout << CYAN << " [" << YELLOW << "1" << RESET << "] Login as " << GREEN << "STAFF" << RESET << endl;
            cout << CYAN << " [" << YELLOW << "2" << RESET << "] Login as " << RED << "ADMIN" << RESET << endl;
            cout << CYAN << " [" << YELLOW << "0" << RESET << "] Exit System" << RESET << endl;
            UIHelper::printLine();
            
            int choice = UIHelper::getIntInput(" >> Select Role: ", 0, 2);
            
            if (choice == 0) {
                cout << GREEN << " >> Shutting down system... Goodbye!" << RESET << endl;
                exit(0);
            }
            
            string user = UIHelper::getStringInput(" >> Username (Enter '0' to cancel): ");
            if (user == "0") continue;
            string pass = UIHelper::getStringInput(" >> Password: ");
            
            if (choice == 1) {
                if (user == "staff" && pass == "123") {
                    cout << GREEN << " >> Login Successful. Loading Dashboard..." << RESET << endl;
                    staffMenu();
                } else {
                    cout << RED << " >> Access Denied. Invalid credentials." << RESET << endl;
                    UIHelper::pressEnterToContinue();
                }
            }
            else if (choice == 2) {
                if (user == "admin" && pass == "admin") {
                    cout << GREEN << " >> Admin Privileges Granted." << RESET << endl;
                    adminControls();
                } else {
                    cout << RED << " >> Access Denied. Invalid credentials." << RESET << endl;
                    UIHelper::pressEnterToContinue();
                }
            }
        }
    }
};

// ==========================================
// 7. ENTRY POINT
// ==========================================
int main() {
    srand(time(0));
    SwiftExEngine app;
    app.login();
    return 0;
}