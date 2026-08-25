using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <cstring>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
const void* actFile;
const void* movFile;

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
  actFile = actorFile;
  movFile = movieFile;
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

int checkOneOffset(const void* actor, const void* offset) {
  int toInt = *(int*)offset;
  char* curr = (char*)actFile + toInt;
  const char* toStr = (char*)actor;
  return strcmp(toStr, curr);
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 
  int numActors = *(int*)actorFile;
  void* find = bsearch(player.c_str(), (char*)actorFile + sizeof(int), numActors, sizeof(int), checkOneOffset);
  if (find == nullptr) return false;
  int foundOffset = *(int*)find;

  int offsetToNumMovies = foundOffset;
  int wholeNameSize = player.size() + 1;
  if (player.size() % 2 == 0) {
    offsetToNumMovies += (player.size() + 2);
    wholeNameSize++;
  } else {
    offsetToNumMovies += (player.size() + 1);
  }

  short numMovies = *(short*)((char*)actorFile + offsetToNumMovies);
  int offsetToMovies = offsetToNumMovies + 2;
  if ((wholeNameSize + 2) % 4 != 0) offsetToMovies += 2;

  for (int i = 0; i < numMovies; i++) {
    int offsetOfMovieFile = *(int*)((char*)actorFile + offsetToMovies + i * sizeof(int));
    film f;
    string title;
    f.title = string((char*)movieFile + offsetOfMovieFile);
    f.year = *((char*)movieFile + offsetOfMovieFile + f.title.size() + 1) + 1900;
    films.push_back(f);
  }

  return true;
}

int checkOneFilm(const void* mov, const void* offset) {
  int toInt = *(int*)offset;
  film f = *(film*)mov;
  film currFilm;
  char* movName = (char*)movFile + toInt;
  currFilm.title = string(movName); 
  char movYear = *((char*)movFile + toInt + string(movName).length() + 1);
  currFilm.year = (int)movYear + 1900;
  if (f == currFilm) return 0;
  if (f < currFilm) return -1;
  return 1;
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 
  int allMovies = *(int*)movieFile;
  void* findMov = bsearch(&movie, (char*)movieFile + sizeof(int), allMovies, sizeof(int), checkOneFilm);
  if (findMov == nullptr) return false;
  int foundMovOffset = *(int*)findMov;

  char* toNumActs = (char*)movFile + foundMovOffset + movie.title.size() + 1 + 1;
  int offsetToActors = foundMovOffset + movie.title.size() + 1 + 1 + 2;
  if ((movie.title.size() + 1 + 1) % 2 == 1) {
    toNumActs++;
    offsetToActors++;
  }
  short numActs = *(short*)toNumActs;
  char* toActors = toNumActs + 2;
  if (offsetToActors % 4 != 0) toActors += 2;

  for (int i = 0; i < numActs; i++) {
    int offsetToOneAct = *(int*)(toActors + i * sizeof(int));
    string foundAct = string((char*)actorFile + offsetToOneAct);
    players.push_back(foundAct);
  }

  return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
