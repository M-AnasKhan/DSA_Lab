#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>

using namespace std;

/* ===== BITWISE FLAGS ===== */
const unsigned int PRIMARY_KEY = 1;
const unsigned int NOT_NULL    = 2;
const unsigned int UNIQUE      = 4;

/* ===== COLUMN ===== */
class Column {
public:
    string name;
    string type;
    unsigned int constraints;

    Column(string n, string t, unsigned int c)
        : name(n), type(t), constraints(c) {}
};

/* ===== ROW ===== */
class Row {
public:
    vector<string> values;
    Row(const vector<string>& v) : values(v) {}
};

/* ===== TABLE ===== */
class Table {
public:
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;

    Table(string name) : tableName(name) {}

    ~Table() {
        for (Row* r : rows)
            delete r;
        rows.clear();
    }
};

/* ===== GLOBAL TABLE ===== */
Table* table = nullptr;

/* ===== VALIDATE INSERT ===== */
bool validateInsert(const vector<string>& values) {
    if (values.size() != table->columns.size())
        return false;

    for (size_t i = 0; i < table->columns.size(); i++) {
        Column& c = table->columns[i];

        if ((c.constraints & NOT_NULL) && values[i].empty())
            return false;

        if (c.constraints & (PRIMARY_KEY | UNIQUE)) {
            for (Row* r : table->rows)
                if (r->values[i] == values[i])
                    return false;
        }
    }
    return true;
}

/* ===== CREATE TABLE ===== */
void createTable(string name) {
    if (table) {
        cout << "Table already exists.\n";
        return;
    }

    table = new Table(name);
    table->columns.push_back(Column("id", "int", PRIMARY_KEY | NOT_NULL));
    table->columns.push_back(Column("name", "string", NOT_NULL));
    table->columns.push_back(Column("age", "int", 0));

    cout << "Table created successfully.\n";
}

/* ===== INSERT ===== */
void insertRow(const vector<string>& values) {
    if (!validateInsert(values)) {
        cout << "Insert failed: constraint violation.\n";
        return;
    }
    table->rows.push_back(new Row(values));
    cout << "Record inserted.\n";
}

/* ===== SELECT ===== */
void selectAll() {
    for (auto& c : table->columns)
        cout << c.name << "\t";
    cout << "\n";

    for (Row* r : table->rows) {
        for (auto& v : r->values)
            cout << v << "\t";
        cout << "\n";
    }
}

/* ===== UPDATE ===== */
void updateWhere(string setCol, string setVal, string whereCol, string whereVal) {
    int s = -1, w = -1;

    for (size_t i = 0; i < table->columns.size(); i++) {
        if (table->columns[i].name == setCol) s = i;
        if (table->columns[i].name == whereCol) w = i;
    }

    if (s == -1 || w == -1) return;

    for (Row* r : table->rows)
        if (r->values[w] == whereVal) {
            r->values[s] = setVal;
            cout << "Record updated.\n";
        }
}

/* ===== DELETE ===== */
void deleteWhere(string col, string val) {
    int idx = -1;
    for (size_t i = 0; i < table->columns.size(); i++)
        if (table->columns[i].name == col)
            idx = i;

    if (idx == -1) return;

    for (auto it = table->rows.begin(); it != table->rows.end();) {
        if ((*it)->values[idx] == val) {
            delete *it;
            it = table->rows.erase(it);
            cout << "Record deleted.\n";
        } else ++it;
    }
}

/* ===== SAVE ===== */
void saveToFile() {
    ofstream file(table->tableName + ".db");

    file << "TABLE " << table->tableName << "\n";
    for (auto& c : table->columns)
        file << c.name << " " << c.type << " " << c.constraints << "\n";

    file << "DATA\n";
    for (Row* r : table->rows) {
        for (auto& v : r->values)
            file << v << " ";
        file << "\n";
    }

    file.close();
    cout << "Data saved.\n";
}

/* ===== LOAD ===== */
void loadFromFile(string name) {
    if (table) delete table;

    ifstream file(name + ".db");
    string tmp;
    file >> tmp >> tmp;

    table = new Table(tmp);
    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line == "DATA") break;
        string n, t; unsigned int c;
        stringstream ss(line);
        ss >> n >> t >> c;
        table->columns.push_back(Column(n, t, c));
    }

    while (getline(file, line)) {
        vector<string> vals;
        string v;
        stringstream ss(line);
        while (ss >> v) vals.push_back(v);
        table->rows.push_back(new Row(vals));
    }

    cout << "Data loaded.\n";
}

/* ===== MAIN ===== */
int main() {
    char buffer[256];
    cout << "Mini Database Engine\n";

    while (true) {
        cout << "> ";
        cin.getline(buffer, 256);
        string cmd(buffer);

        if (cmd == "EXIT") break;

        stringstream ss(cmd);
        string w;
        ss >> w;

        if (w == "CREATE") {
            ss >> w >> w;
            createTable(w);
        }
        else if (w == "INSERT") {
            vector<string> vals;
            ss >> w >> w >> w;
            string v;
            while (ss >> v) vals.push_back(v);
            insertRow(vals);
        }
        else if (w == "SELECT") selectAll();
        else if (w == "UPDATE") {
            string t, s, se, wh, we;
            ss >> t >> s >> se >> wh >> we;
            updateWhere(se.substr(0,se.find('=')),
                        se.substr(se.find('=')+1),
                        we.substr(0,we.find('=')),
                        we.substr(we.find('=')+1));
        }
        else if (w == "DELETE") {
            ss >> w >> w >> w;
            string exp; ss >> exp;
            deleteWhere(exp.substr(0,exp.find('=')),
                        exp.substr(exp.find('=')+1));
        }
        else if (w == "SAVE") saveToFile();
        else if (w == "LOAD") { ss >> w; loadFromFile(w); }
        else cout << "Invalid command.\n";
    }

    delete table;
    return 0;
}