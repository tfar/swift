4.0-beta1 ( 2016-07-15 )
------------------------
- Moved code-base to C++11
    - Use C++11 threading instead of Boost.Thread library
    - Use C++11 smart pointers instead of Boost's
- Migrated from Boost.Signals to Boost.Signals2
- Build without warnings on our CI platforms
- General cleanup like remove of superflous files and #include statements. This means header files that previously were included implictly need to be explicitly included now
- Support IPv6 addresses in URLs
- Changed source code style to use soft tabs (4 spaces wide) instead of hard tabs. Custom patches for Swiften will need to be reformatted accordingly
- Require a TLS backend for building
- Update 3rdParty/lcov to version 1.12
- Fix several possible race conditions and other small bugs