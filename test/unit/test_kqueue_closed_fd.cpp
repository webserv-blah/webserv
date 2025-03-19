#include "TestFramework.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/event.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

std::string filterToString(int filter) {
    switch (filter) {
        case EVFILT_READ:    return "EVFILT_READ";
        case EVFILT_WRITE:   return "EVFILT_WRITE";
        case EVFILT_AIO:     return "EVFILT_AIO";
        case EVFILT_VNODE:   return "EVFILT_VNODE";
        case EVFILT_PROC:    return "EVFILT_PROC";
        case EVFILT_SIGNAL:  return "EVFILT_SIGNAL";
        case EVFILT_TIMER:   return "EVFILT_TIMER";
        case EVFILT_USER:    return "EVFILT_USER";
        default:            return "UNKNOWN(" + std::to_string(filter) + ")";
    }
}

std::string flagsToString(uint16_t flags) {
    std::string result;
    
    if (flags & EV_EOF)     result += "EV_EOF ";
    if (flags & EV_ERROR)   result += "EV_ERROR ";
    if (flags & EV_ONESHOT) result += "EV_ONESHOT ";
    if (flags & EV_CLEAR)   result += "EV_CLEAR ";
    if (flags & EV_RECEIPT) result += "EV_RECEIPT ";
    if (flags & EV_DISPATCH) result += "EV_DISPATCH ";
    if (flags & EV_DISABLE) result += "EV_DISABLE ";
    
    return result.empty() ? "0" : result;
}

std::string eventDataToString(const struct kevent& event) {
    // First check if this is an error event
    if (event.flags & EV_ERROR) {
        // For error events, data contains the error code
        return std::string("error: ") + strerror(static_cast<int>(event.data)) 
              + " (" + std::to_string(event.data) + ")";
    }
    
    // Interpret data based on filter type for non-error events
    if (event.filter == EVFILT_READ || event.filter == EVFILT_WRITE) {
        return std::to_string(event.data) + " bytes";
    } else if (event.filter == EVFILT_VNODE) {
        std::string result;
        intptr_t data = event.data;
        if (data & NOTE_DELETE) result += "NOTE_DELETE ";
        if (data & NOTE_WRITE) result += "NOTE_WRITE ";
        if (data & NOTE_EXTEND) result += "NOTE_EXTEND ";
        if (data & NOTE_ATTRIB) result += "NOTE_ATTRIB ";
        if (data & NOTE_LINK) result += "NOTE_LINK ";
        if (data & NOTE_RENAME) result += "NOTE_RENAME ";
        if (data & NOTE_REVOKE) result += "NOTE_REVOKE ";
        return result.empty() ? "0" : result;
    }
    // Default to raw integer for other filter types
    return std::to_string(event.data);
}

// TestSuite declaration
TEST_SUITE(KqueueClosedFD)

// Test case to reproduce kqueue reporting events for closed file descriptors
TEST_CASE(EventsAfterClose, KqueueClosedFD) {
    // 1. Create a kqueue instance
    int kq = kqueue();
    ASSERT_TRUE(kq >= 0);
    std::cout << "Created kqueue fd: " << kq << std::endl;
    
    // 2. Create a socket pair (this simulates a client-server connection)
    int sockets[2];
    ASSERT_TRUE(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) >= 0);
    
    int serverSocket = sockets[0];
    int clientSocket = sockets[1];
    std::cout << "Created socketpair: serverSocket=" << serverSocket << ", clientSocket=" << clientSocket << std::endl;
    
    // 3. Register the client socket for read events in kqueue
    struct kevent change;
    EV_SET(&change, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    ASSERT_TRUE(kevent(kq, &change, 1, NULL, 0, NULL) >= 0);
    std::cout << "Registered clientSocket for READ events" << std::endl;
    
    // 4. Write data to the server socket (this will trigger a read event on the client socket)
    const char* message = "Test Message";
    ASSERT_TRUE(write(serverSocket, message, strlen(message)) > 0);
    std::cout << "Wrote test message to serverSocket" << std::endl;
    
    // 5. Check for events (should see a read event for clientSocket)
    std::vector<struct kevent>	eventList_(1024);
    struct timespec timeout = {1, 0}; // 1 second timeout
    
    std::cout << "Checking for events" << std::endl;
    int numEvents = kevent(kq, NULL, 0, &eventList_[0], 1024, &timeout);
    if (numEvents > 0) {
        std::cout << "Received " << numEvents << " events" << std::endl;
        // Fix: Loop through all received events
        for (int i = 0; i < numEvents; i++) {
            std::cout << "Event for fd: " << eventList_[i].ident 
                      << ", filter: " << filterToString(eventList_[i].filter)
                      << ", flags: " << flagsToString(eventList_[i].flags)
                      << ", data: " << eventDataToString(eventList_[i]) << std::endl;
        }
    } else {
        std::cout << "No events detected" << std::endl;
    }
    
    // 6. Close the client socket
    close(clientSocket);
    std::cout << "Closed clientSocket fd: " << clientSocket << std::endl;
    
    // 7. Set up the deletion changes, but don't apply them yet
    struct kevent changes[1];
    std::vector<struct kevent> changedEvents_;

    // Fix: Use clientSocket instead of fd (which was undefined)
    std::cout << "Deleting EVFILT_READ event for clientSocket" << std::endl;
    EV_SET(&changes[0], clientSocket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    // std::cout << "Deleted EVFILT_WRITE event for clientSocket" << std::endl;
    // EV_SET(&changes[1], clientSocket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    changedEvents_.insert(changedEvents_.end(), changes, changes + 1);
    int numChanges = changedEvents_.size();

    // 8. Check if we still get events for the closed file descriptor
    // This call both processes our deletion requests and returns any events
    std::cout << "Checking for events" << std::endl;
    numEvents = kevent(kq, &changedEvents_[0], numChanges, &eventList_[0], 1024, &timeout);

    if (numEvents > 0) {
        // std::cout << "ISSUE DETECTED: Still receiving events after socket close!" << std::endl;
        std::cout << "Received " << numEvents << " events" << std::endl;
        // Fix: Loop through all received events
        for (int i = 0; i < numEvents; i++) {
            std::cout << "Event for fd: " << eventList_[i].ident 
                      << ", filter: " << filterToString(eventList_[i].filter)
                      << ", flags: " << flagsToString(eventList_[i].flags)
                      << ", data: " << eventDataToString(eventList_[i]) << std::endl;
        }
    } else {
        std::cout << "No events detected" << std::endl;
    }
    
    // 11. Clean up the server socket and kqueue
    close(serverSocket);
    close(kq);
    
    // Test passes if we reach this point without crashes
    ASSERT_TRUE(true);
}