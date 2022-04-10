#ifndef BUGME_ERROR_HH
#define BUGME_ERROR_HH

namespace bugme {

typedef const unsigned int errno_t;
typedef const unsigned int exitno_t;

namespace exit {
inline exitno_t EXIT_ERROR = 0;
inline exitno_t EXIT_WINDOW_CLOSE = 1;
inline exitno_t EXIT_SIGINT = 2;
}  // namespace exit

namespace error {
inline errno_t SUCCESS = 0;
}  // namespace error

}  // namespace bugme

#endif
