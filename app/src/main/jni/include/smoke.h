#ifndef SMOKE_H
#define SMOKE_H

#include "galois.h"
#include <string.h>
#include <vector>
#include <list>
#include <tuple>
#include <stdio.h>
#include <set>
#include <time.h>
#include <map>
#include <assert.h>
#include <uv.h>

#ifdef SMOKE_SERVER
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#endif

void close_and_free_cb(uv_handle_t* handle);

struct sender_context
{
	uv_udp_t * socket;
	sockaddr_in addr;
	uint64_t last_outgoing_time;
};

struct send_chunk
{
	uint32_t generation_id;
	uint8_t send_count;
	uint64_t timestamp;
};

struct encrypt_context
{
	uv_loop_t * loop;
	uint32_t max_generation_size;
	uint32_t generation_biggest_packet_size;
	uint32_t generation_id;
	std::list<std::vector<uint8_t>> original_packets;
	std::list<std::vector<uint8_t>> backup_packets;
	std::map<uint32_t, send_chunk> send_table;
	sender_context sender;
	double estimated_drop_rate;
	bool first_packet_of_generation;
	uint32_t round_trip_time;
	uint32_t foam_count;
	uv_timer_t * flush_timer;
	encrypt_context(const encrypt_context& it) = delete;
	encrypt_context(){}
};

struct decrypt_backup
{
	std::vector<uint8_t> buffer;
	uint8_t coefficient_index;
	decrypt_backup(const decrypt_backup& it) = delete;
	decrypt_backup(){}
};

struct decrypt_context;

struct decrypt_generation
{
	uint32_t id;
	uint32_t size;
	uint32_t backup_size;
	std::set<uint32_t> received_main_packets;
	std::set<uint32_t> received_backups;
	std::vector<uint8_t> main_buffer;
	std::list<decrypt_backup> backups;
	uint64_t last_accepted_time;
	uv_timer_t * flush_timer;
	decrypt_context * ctx;
	decrypt_generation(const decrypt_generation& it) = delete;
	decrypt_generation(){}
	~decrypt_generation()
	{
		uv_close((uv_handle_t*)flush_timer,close_and_free_cb);
	}
};

struct recv_chunk
{
	uint32_t generation_id;
	uint8_t recv_count;
	uint64_t timestamp;
};

struct decrypt_context
{
	uv_loop_t * loop;
	std::map<uint32_t,decrypt_generation> generation_queue;
	std::set<std::pair<uint32_t,uint32_t>> already_sent;
	sender_context sender;
	uint64_t last_accepted_time;
	std::map<uint32_t,recv_chunk> recv_table;
	uint64_t round_trip_time;
	decrypt_context(const decrypt_context& it) = delete;
	decrypt_context(){}
};

struct client_info
{
	decrypt_context decrypt_ctx;
	encrypt_context encrypt_ctx;
	uint64_t last_incoming_time;
	uint32_t client_version;
	uint32_t big_packets_at_start;
	std::vector<uint8_t> session_key;
	sockaddr_in ip_hop_addr;
	client_info(const client_info& it) = delete;
	client_info(){}
};

struct smoke_server_client_socket
{
	uv_udp_t * socket;
	sockaddr_in addr;
};

struct smoke_server_context
{
	sockaddr_in ovpn_server_addr;
	std::list<smoke_server_client_socket> client_sockets;
	uv_loop_t * loop;
	uv_timer_t * per_second_handler;
	std::list<client_info> clients;
	smoke_server_context(const smoke_server_context& it) = delete;
	smoke_server_context(){}
};

enum connection_state_t
{
	connection_not_established,
	connection_established,
	connection_lost
};

struct smoke_client_context
{
	sender_context hop_ctx;
	int hops_left;
	connection_state_t connection_state;
	sockaddr_in local_addr,smoke_server_addr;
	uv_loop_t * loop;
	uv_timer_t * per_second_handler;
	uv_timer_t * handshake_timeout_handler;
	uv_timer_t * regular_hop_handler;
	uv_timer_t * hop_timeout_handler;
	uv_timer_t * connection_timeout_handler;
	client_info connection;
	smoke_client_context(const smoke_client_context& it) = delete;
	smoke_client_context(){}
};

void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf);

void send_cb(uv_udp_send_t* req, int status);

void close_and_free_cb(uv_handle_t* handle);

void sender_send(sender_context * sender, uint64_t time, uv_buf_t * buf);

void rlnc_encode_backup(std::list<std::tuple<uint32_t,uint8_t*,uint32_t>> packets, std::list<std::tuple<uint32_t,uint8_t*>> backup_packets);

void rlnc_encrypt_queue_flush(encrypt_context * ctx);

void rlnc_decrypt_try_to_solve(decrypt_context * ctx, decrypt_generation * gen);

#ifdef SMOKE_SERVER
void server_event_message_from_ovpn_server_cb(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags);
#endif

#ifdef SMOKE_QT
#include <QThread>

class smoke : public QThread
{
	Q_OBJECT
signals:
	void connection_established(int port);
	void connection_refused();
	void connection_timeout();
public slots:
	void close_smoke();
protected:
	void run();
#else
class smoke
{
public:
	void close_smoke();
	void run();
#endif
public:
	smoke();
	void smoke_main();
#ifdef SMOKE_SERVER
	void server_event_message_from_client(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags);
	void server_event_message_from_ovpn_server(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags);
	void server_event_every_second(uv_timer_t *t);
	void start_server(int local_udp_port, int ovpn_server_udp_port, std::string ovpn_server_ip);
	client_info* server_init_client(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags);
private:
	smoke_server_context ctx;
#elif SMOKE_CLIENT
	void client_request_hop();
	void client_hop_failed();
	void client_event_message_from_client(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, uint32_t flags);
	void client_event_message_from_server(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, uint32_t flags);
	void client_handshake_timeout(uv_timer_t *t);
	void client_event_every_second(uv_timer_t *t);
	void start_client(int local_udp_port, int smoke_server_udp_port, std::string smoke_server_ip);
	void connection_timeout_handler(uv_timer_t *t);
	void regular_hop_handler(uv_timer_t *t);
	void hop_timeout_handler(uv_timer_t *t);

	int local_port;
	std::vector<std::pair<std::string,int>> remote_hosts;
	int remote_host_index;
	int request_big_packets;
	int hop_interval;
	int hop_timeout;
	int hop_attempts;
	int connection_timeout;
	int handshake_timeout;
private:
	smoke_client_context ctx;
#else
#error compile with -DSMOKE_SERVER or -DSMOKE_CLIENT
#endif
};

#endif // SMOKE_H
