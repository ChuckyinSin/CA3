/*
 * Skeleton code for use with Computer Architecture 2023 assignment 3,
 * LIACS, Leiden University.
 */

#include "postcode.h"

#include <unordered_map>
#include <algorithm>

#include <iostream>

static void
run_query_scan(const std::vector<Record> &postcode_table,
               std::vector<Record> &result_table,
               const size_t blockSize)
{
  for (auto &result : result_table)
    {
      auto it = std::find(postcode_table.begin(), postcode_table.end(),
                          result.postcode);
      if (it != postcode_table.end())
        {
          strncpy(result.street.data(), it->street.data(), Record::STREET_LEN);
          strncpy(result.city.data(), it->city.data(), Record::CITY_LEN);
        }
    }
}

static void
run_query_hash(const std::vector<Record> &postcode_table,
               std::vector<Record> &result_table,
               const size_t blockSize)
{
  std::unordered_map<Record::Postcode, size_t> hash;

  /* Setting the number of buckets in advance has a positive effect
   * on performance. The hash function provided is near perfect.
   */
  hash.rehash(postcode_table.size());

  for (size_t i = 0; i < postcode_table.size(); ++i)
    hash[postcode_table[i].postcode] = i;

  for (auto &result : result_table)
    {
      auto it = hash.find(result.postcode);
      if (it != hash.end())
        {
          strncpy(result.street.data(), postcode_table[it->second].street.data(), Record::STREET_LEN);
          strncpy(result.city.data(), postcode_table[it->second].city.data(), Record::CITY_LEN);
        }
    }
}

/* Add additional variants here. In support/postcode.h you can find
 * a binary_search routine you can use. You can add the new routines
 * in the table searchModes below to test and benchmark these.
 */

struct SearchMode{
  std::string name;
  RunQueryFunction func;
} searchModes[] = {
  {"scan", run_query_scan},
  {"hash", run_query_hash},

  /* This must be the last entry. */
  {"", NULL}
};

int
usage (int argc, char **argv)
{
  std::cerr << "Usage: " << argv[0]
            << " <postcode_file> <query_file> <n_repeat>" << std::endl;
  std::cerr << "       " << argv[0]
            << " <postcode_file> <query_file> -t <mode>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Options: " << std::endl;
  std::cerr << "  -t <mode>  Use the search function <mode>, and print the" << std::endl
            << "             results of the query to stdout." << std::endl;
  return EXIT_FAILURE;
}

int
main (int argc, char **argv)
{
  if (argc < 4)
    return usage(argc, argv);

  std::string postcode_filename(argv[1]);
  std::string query_filename(argv[2]);

  std::string testMode = "";
  long int n_repeat = 0;

  if (std::string(argv[3]) == "-t")
    {
      if(argc < 5)
        return usage(argc, argv);

      testMode = argv[4];
    }
  else if (argv[3][0] == '-')
    {
      std::cerr << "Unknown option " << argv[3] << "." << std::endl;
      return EXIT_FAILURE;
    }
  else
    {
      n_repeat = strtol(argv[3], NULL, 10);
      if (errno > 0 || n_repeat == 0)
        {
          std::cerr << "Invalid repeat amount <n_repeat>." << std::endl;
          return EXIT_FAILURE;
        }
    }

  /* Load files */
  std::vector<Record> postcode_table;
  if (! read_postcode_table(postcode_filename, postcode_table))
    return EXIT_FAILURE;

  std::cerr << "loaded " << postcode_table.size() << " entries." << std::endl;

  std::vector<Record> result_table_blank;
  if (! read_query(query_filename, result_table_blank))
    return EXIT_FAILURE;

  bool result = false;
  if (testMode != "")
    {
      /* Run single function and print results */
      SearchMode *mode = &searchModes[0];
      for(; mode->func != NULL; mode++)
        {
          if(mode->name == testMode)
            break;
        }
      if(mode->func == NULL)
        {
          std::cerr << "No mode \"" << testMode << "\"" << std::endl;
          return EXIT_FAILURE;
        }

      result = test_query_function(mode->func, 0,
                                   postcode_table, result_table_blank);
      return result ? EXIT_SUCCESS : EXIT_FAILURE;
    }

  /* Run all functions */
  SearchMode *mode = &searchModes[0];
  for(; mode->func != NULL; mode++)
    {
      result |= bench_query_function(mode->func,
                                     mode->name, n_repeat, 0,
                                     postcode_table, result_table_blank,
                                     postcode_filename, query_filename);
    }
  return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
