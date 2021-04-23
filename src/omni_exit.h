#ifndef OMNI_EXIT_H
#define OMNI_EXIT_H

/*
 * Use this when you hit an unrecoverable error (ie all of them) to show a popup
 * message to the user and then exit with a code of 1.
 */
void displayErrorAndExit(const char *format, ...);

/*
 * Adds a method to catch SIGSEGV signals and display a message to the user
 * before reinstalling the original handler and re-raising the exception.
 * 
 * Will only work once, to prevent overwriting the default handler if called too many times
 * But then again, you should only call once.
 */
void installSegfaultHandler();

#endif
