#include "trace_perf.h"
#include "detectgdb.h"
#include "shared_state.h"

#include <vector>
#include <fstream>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>

#include <sys/stat.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // CreateDirectory
#include <Winsock2.h> // gethostname
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h> // gethostname
#endif

bool PRINT_ATTEMPTED_DATABASE_FILES = true;

using namespace std;

class performance_traces {
private:
    struct Entry {
        string info;
        double elapsed;
    };

    map<string, vector<Entry>> entries;
    vector<string> database_paths;

    vector<string> get_database_names(string sourcefilename);
    void load_db(map<string, map<string, double>>& dbs, string sourcefilename);
    void compare_to_db(map<string, double> &db, const vector<Entry>& entries, string sourcefilename);

    void compare_to_db();
    void dump_entries();

    static void read_database(map<string, double>& db, string filename);
    static void dump_entries(const vector<Entry>& entries, string sourcefilaname);

public:
    performance_traces();
    ~performance_traces();

    void log(string filename, string info, double elapsed) {
        size_t i = filename.find_last_of ('/');
        if (string::npos != i)
            filename = filename.substr (i+1);

        entries[filename].push_back(Entry{info, elapsed});
    }

    void add_path(string path) {
        database_paths.push_back (path);
    }
};

static shared_state<performance_traces> traces {new performance_traces};


performance_traces::
        performance_traces()
{
    add_path ("trace_perf");
}


performance_traces::
        ~performance_traces()
{
    fflush (stdout);
    fflush (stderr);

    compare_to_db ();
    dump_entries ();
}


void performance_traces::
        load_db(map<string, map<string, double>>& dbs, string sourcefilename)
{
    if (dbs.find (sourcefilename) != dbs.end ())
        return;

    map<string, double> db;

    vector<string> dbnames = get_database_names(sourcefilename);
    for (unsigned i=0; i<dbnames.size (); i++)
        read_database(db, dbnames[i]);

    if (db.empty ())
        cerr << "Couldn't read databases in folder trace_perf" << endl;

    dbs[sourcefilename] = db;
}


void performance_traces::
        compare_to_db()
{
    map<string, map<string, double>> dbs;
    for (auto a = entries.begin (); a!=entries.end (); a++)
    {
        string sourcefilename = a->first;
        load_db(dbs, sourcefilename);
    }

    for (auto a = dbs.begin (); a!=dbs.end (); a++)
    {
        string sourcefilename = a->first;
        map<string, double>& db = a->second;
        vector<Entry>& entries = this->entries[sourcefilename];

        compare_to_db(db, entries, sourcefilename);
    }

    for (auto i = dbs.begin (); i!=dbs.end (); i++)
    {
        map<string, double>& db = i->second;

        for (auto j = db.begin (); j!=db.end (); j++)
            cerr << i->first << ": Missing trace_perf test \'" << j->first  << "\'" << endl;
    }
}


void performance_traces::
        compare_to_db(map<string, double> &db, const vector<Entry>& entries, string sourcefilename)
{
    bool expected_miss = false;
    for (unsigned i=0; i<entries.size (); i++)
    {
        string info = entries[i].info;
        double elapsed = entries[i].elapsed;

        double expected = -1;

        auto j = db.find (info);
        if (j != db.end ())
        {
            expected = j->second;
            db.erase (j);
        }

        if (elapsed > expected)
        {
            if (!expected_miss) {
                cerr << endl << sourcefilename << " wasn't fast enough ..." << endl;
                if (PRINT_ATTEMPTED_DATABASE_FILES) {
                    vector<string> dbnames = get_database_names(sourcefilename);
                    for (unsigned i=0; i<dbnames.size (); i++)
                        cerr << dbnames[i] << endl;
                }
            }

            cerr << endl;
            cerr << info << endl;
            cerr << elapsed << " > " << expected << endl;
            expected_miss = true;
        }
    }

    if (expected_miss)
        cerr << endl;
}


void performance_traces::
        dump_entries()
{
    for (auto i=entries.begin (); i!=entries.end (); i++)
        dump_entries (i->second, i->first);
}


void performance_traces::
        dump_entries(const vector<Entry>& entries, string sourcefilaname)
{
    // requires boost_filesystem
    //boost::filesystem::create_directory("trace_perf");
    //boost::filesystem::create_directory("trace_perf/dump");

#ifdef _WIN32
    // ignore errors
    CreateDirectory(L"trace_perf", NULL);
    CreateDirectory(L"trace_perf/dump", NULL);
#else
    // require posix
    mkdir("trace_perf", S_IRWXU|S_IRGRP|S_IXGRP);
    mkdir("trace_perf/dump", S_IRWXU|S_IRGRP|S_IXGRP);
#endif

    int i=0;
    string filename;
    while (true) {
        stringstream ss;
        ss << "trace_perf/dump/" << sourcefilaname << ".db" << i;
        filename = ss.str ();
        ifstream file(filename);
        if (!file)
            break;
        i++;
    }

    ofstream o(filename);
    if (!o)
        cerr << "Couldn't dump performance entries to " << filename << endl;

    for (unsigned i=0; i<entries.size (); i++)
    {
        if (0 < i)
            o << endl;

        o << entries[i].info << endl
          << entries[i].elapsed << endl;
    }
}


void performance_traces::
        read_database(map<string, double>& db, string filename)
{
    std::string info;
    double expected;

    ifstream a(filename);
    if (!a.is_open ())
        return;

    while (!a.eof() && !a.fail())
    {
        getline(a,info);
        a >> expected;
        db[info] = expected;

        getline(a,info); // finish line
        getline(a,info); // read empty line
    }
}


vector<string> performance_traces::
        get_database_names(string sourcefilename)
{
    string hostname("unknown");
    hostname.reserve (256);
    gethostname(&hostname[0], hostname.capacity ());

    vector<string> config;

#if defined(__APPLE__)
    config.push_back ("-apple");
#elif defined(_WIN32)
    config.push_back ("-windows");
#endif

#ifdef _DEBUG
    config.push_back ("-debug");
#endif

    if (DetectGdb::is_running_through_gdb())
        config.push_back ("-gdb");

    vector<string> db;
    for (int i=0; i<(1 << config.size ()); i++)
    {
        string perm;
        for (unsigned j=0; j<config.size (); j++)
        {
            if ((i>>j) % 2)
                perm += config[j];
        }
        db.push_back (sourcefilename + ".db" + perm);
    }

    size_t n = db.size();
    if (!hostname.empty ())
        for (size_t i=0; i<n; i++)
            db.push_back (hostname + "/" + db[i]);

    vector<string> dbfiles;
    for (unsigned j=0; j<database_paths.size (); j++)
        for (unsigned i=0; i<db.size (); i++)
            dbfiles.push_back (database_paths[j] + "/" + db[i]);

    return dbfiles;
}


trace_perf::trace_perf(const char* filename, const string& info)
    :
      filename(filename)
{
    reset(info);
}


trace_perf::
        ~trace_perf()
{
    reset();
}


void trace_perf::
        reset()
{
    double d = timer.elapsed ();
    if (!info.empty ())
        traces->log (filename, info, d);
}


void trace_perf::
        reset(const string& info)
{
    reset();

    this->info = info;
    this->timer.restart ();
}


void trace_perf::
        add_database_path(const std::string& path)
{
    traces->add_path(path);
}
