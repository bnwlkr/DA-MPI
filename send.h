#ifndef SEND_H
#define SEND_H


#endif



/*
 * The solution to the migration send problem is this: in send, shift the information about the intended recipient rank (and sender) into somehwere else in the message, make the translation, and send via the proc.
 *  in RECV, if a message is recvd that wasnt intended for this rank (wrong process) forward the message on with the source information in the message, and RECV again. In our special recv, re-allocate information into the request parameter so that it looks like a normal receive.
 *  everything happens under the hood.
 *
 */
