#include <unordered_map>
#include <iostream>
#include <vector>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

struct Node {
    string key;
    int value;

    Node() {
        this->key = "";
        this->value = -1;
    }

    Node(string &key, int &value) {
        this->key = key;
        this->value = value;
    }
};

class HashTable {
private:
    Node **arr;
    int capacity;
    int size;

    void initialize() {
        this->arr = new Node *[this->capacity];
        for (int i = 0; i < this->capacity; ++i) {
            this->arr[i] = new Node();
        }
    }

    void freeMemory() {
        for (int i = 0; i < this->capacity; ++i) {
            delete this->arr[i];
        }

        delete[] this->arr;
    }

    void _resize() {
        int oldCapacity = this->capacity;

        if (2 * this->size > this->capacity) {
            this->capacity *= 2;
        } else if (8 * this->size < this->capacity) {
            this->capacity /= 2;
        } else {
            return;
        }

        Node **copyArr = new Node *[oldCapacity];
        for (int i = 0; i < oldCapacity; ++i) {
            copyArr[i] = this->arr[i];
        }

        this->arr = new Node *[this->capacity];
        this->size = 0;
        for (int i = 0; i < this->capacity; ++i) {
            this->arr[i] = new Node();
        }

        for (int i = 0; i < oldCapacity; ++i) {
            if (copyArr[i]->value != -1) {
                add(copyArr[i]->key, true, copyArr[i]->value);
            }
        }
    }

    void _remove(int hashVal) {
        int index = 1;
        bool replaced = false;

        Node *next = this->arr[hashVal + index];
        if (hashVal + index == this->capacity) {
            this->arr[hashVal] = new Node();
            return;
        }

        if (next->value == -1) {
            this->arr[hashVal] = new Node();
            return;
        }

        while (this->arr[hashVal + index]->value != -1 && hashVal + index < this->capacity) {
            int nextHash = hash(next->key, this->capacity - 1);
            if (nextHash <= hashVal) {
                replaced = true;
                this->arr[hashVal] = next;
            }

            if (replaced) {
                _remove(nextHash);
            }

            ++index;
        }
    }

    static int hash(string &key, int mod) {
        const int p = 53;
        if (mod % 2 == 0) mod -= 1;

        int hashVal = 0;
        int pPower = 1;

        for (auto &k : key) {
            if (k >= 'a' && k <= 'z') {
                hashVal = (hashVal + (k - 'a' + 1) * pPower) % mod;
                pPower = (pPower * p) % mod;
            } else if (k >= 'A' && k <= 'Z') {
                hashVal = (hashVal + (k - 'A' + 1) * pPower) % mod;
                pPower = (pPower * p) % mod;
            }
        }

        return hashVal;
    }

public:
    HashTable() {
        this->size = 0;
        this->capacity = 16;
        initialize();
    }

    HashTable(const HashTable &table) {
        this->size = table.size;
        this->capacity = table.capacity;

        freeMemory();
        initialize();

        for (int i = 0; i < this->size; ++i) {
            this->arr[i]->value = table.arr[i]->value;
            this->arr[i]->key = table.arr[i]->key;
        }
    }

    ~HashTable() {
        freeMemory();
    }

    void add(string &key, bool isResizing, int val = -1) {
        int hashValue = hash(key, this->capacity - 1);

        if (this->arr[hashValue]->value == -1) {
            if (isResizing) {
                this->arr[hashValue]->key = key;
                this->arr[hashValue]->value = val;
                this->size++;
                return;
            }

            this->arr[hashValue]->key = key;
            this->arr[hashValue]->value = 1;
            this->size++;
            _resize();
        } else {
            if (this->arr[hashValue]->key.compare(key) == 0) {
                this->arr[hashValue]->value++;
            } else {
                int index = 1;
                bool foundInitializedCell = false;
                while (index + hashValue < this->capacity) {
                    if (this->arr[hashValue + index]->value != -1
                        && this->arr[hashValue + index]->key.compare(key) != 0) {
                        index++;
                    } else if (this->arr[hashValue + index]->value != -1
                               && this->arr[hashValue + index]->key.compare(key) == 0) {
                        foundInitializedCell = true;
                        break;
                    } else
                        break;
                }

                if (index + hashValue == this->capacity) {
                    hashValue = 0;
                    index = 0;

                    while (index < this->capacity) {
                        if (this->arr[index]->value != -1
                            && this->arr[index]->key.compare(key) != 0) {
                            index++;
                        } else if (this->arr[index]->value != -1
                                   && this->arr[index]->key.compare(key) == 0) {
                            foundInitializedCell = true;
                            break;
                        } else
                            break;
                    }
                }

                if (isResizing && !foundInitializedCell) {
                    this->arr[index + hashValue]->key = key;
                    this->arr[index + hashValue]->value = val;
                    this->size++;
                    return;
                }

                if (foundInitializedCell) {
                    this->arr[hashValue + index]->value++;
                    return;
                }

                this->arr[hashValue + index]->key = key;
                this->arr[hashValue + index]->value = 1;
                this->size++;
                if (!isResizing) _resize();
            }
        }
    }

    //return pair<foundOrNot, pair<occurrences, index>>
    pair<bool, pair<int, int>> search(string &key) {
        int hashVal = hash(key, this->capacity - 1);

        if (this->arr[hashVal]->value != -1 && this->arr[hashVal]->key.compare(key) == 0) {
            return {true, {this->arr[hashVal]->value, hashVal}};
        } else if (this->arr[hashVal]->key != key) {
            int index = 1;
            bool found = false;

            while (index + hashVal < this->capacity) {
                if ((this->arr[hashVal + index]->key).compare(key) == 0) {
                    found = true;
                }

                if (this->arr[hashVal + index]->value == -1) {
                    found = false;
                    break;
                }

                if (found) break;
                else index++;
            }

            if (index + hashVal == this->capacity && !found) {
                index = 0;
                hashVal = 0;
                while (index < this->capacity) {
                    if (this->arr[index]->key.compare(key) == 0) {
                        found = true;
                    }

                    if (this->arr[index]->value == -1) {
                        found = false;
                        break;
                    }

                    if (found) break;
                    else index++;
                }
            }

            if (found) return {true, {this->arr[hashVal + index]->value, hashVal + index}};
            else return {false, {-1, -1}};
        }

        return {false, {-1, -1}};
    }

    bool remove(string &key) {
        pair<bool, pair<int, int>> res = search(key);
        if (res.first) {
            _remove(res.second.second);
            this->size--;
            _resize();
            return true;
        }

        return false;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);

    chrono::duration<double, milli> duration;

    auto start = chrono::steady_clock::now();
    HashTable table = HashTable();
    auto stop = chrono::steady_clock::now();

    duration = stop - start;
    cout << "Time to create the hash table: " << duration.count() << endl;

    start = chrono::steady_clock::now();
    unordered_map<string, int> um;
    stop = chrono::steady_clock::now();

    duration = stop - start;
    cout << "Time to create the hash table (c++): " << duration.count() << endl;

    cout << "Enter a number of words in the text:" << endl;
    int numberOfWordsInString;
    cin >> numberOfWordsInString;

    cout << "Enter the text: " << endl;
    string text;
    vector<string> arr(numberOfWordsInString);
    int index = 0;
    int copyNumber = numberOfWordsInString;
    while (copyNumber--) {
        cin >> text;
        arr[index++] = text;
    }

    start = chrono::steady_clock::now();
    for (int j = 0; j < numberOfWordsInString; ++j) {
        table.add(arr[j], false);
    }
    stop = chrono::steady_clock::now();

    duration = stop - start;
    cout << "Time to add all keys in the hash table: " << duration.count() << endl;

    start = chrono::steady_clock::now();
    for (int j = 0; j < numberOfWordsInString; ++j) {
        if (um.count(arr[j])) um[arr[j]]++;
        else um[arr[j]] = 1;
    }
    stop = chrono::steady_clock::now();

    duration = stop - start;
    cout << "Time to add all keys in the hash table (c++): " << duration.count() << endl;

    cout << "Enter a word to search in the text: " << endl;
    text = "";
    cin >> text;

    start = chrono::steady_clock::now();
    pair<bool, pair<int, int>> res = table.search(text);
    stop = chrono::steady_clock::now();
    duration = stop - start;
    cout << "Time to search for a key in the hash table: " << duration.count() << endl;

    start = chrono::steady_clock::now();
    auto el = um.find(text);
    stop = chrono::steady_clock::now();
    duration = stop - start;
    cout << "Time to search for a key in the hash table (c++): " << duration.count() << endl;

    if (res.first) {
        cout << "Number of occurrences in the text: " << res.second.first << endl;
    } else {
        cout << "Word not found!" << endl;
    }

    if (el != um.end()) {
        cout << "Number of occurrences in the text (c++): " << el->second << endl;
    } else {
        cout << "Word not found!" << endl;
    }

    cout << "Enter a word to delete from the text:" << endl;
    text = "";
    cin >> text;

    start = chrono::steady_clock::now();
    bool deleted = table.remove(text);
    stop = chrono::steady_clock::now();
    duration = stop - start;
    cout << "Time to delete a key in the hash table: " << duration.count() << endl;

    if (deleted) {
        cout << "Deleted!" << endl;
    } else {
        cout << "Not deleted!" << endl;
    }

    cout << "Enter a word to delete from the text (c++):" << endl;
    text = "";
    cin >> text;

    start = chrono::steady_clock::now();
    um.erase(text);
    stop = chrono::steady_clock::now();
    duration = stop - start;
    cout << "Time to delete a key in the hash table (c++): " << duration.count() << endl;

    return 0;
}