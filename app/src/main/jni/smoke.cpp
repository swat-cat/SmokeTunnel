#include "smoke.h"

#define SMOKE_LOG_LEVEL 2

#ifdef SMOKE_QT
#define smoke_printf(n,...) {if(SMOKE_LOG_LEVEL>=n) qDebug(__VA_ARGS__);}
#else
#define smoke_printf(n,...) {if(SMOKE_LOG_LEVEL>=n) printf(__VA_ARGS__);}
#endif

#ifdef _WIN32
#define bzero(x,y) memset(x,0,y)
#endif

extern uint8_t coefficient_table[];

#define coefficient_table_row_size 256

#define SMOKE_PROTOCOL_VERSION_TOO_SMALL	0
#define SMOKE_PROTOCOL_VERSION			2//added packet identifiers for ip hop, server returns a session string to client during handshake
#define SMOKE_PROTOCOL_VERSION_TOO_BIG		3

#define ensure(x) {if(!(x)){free(buf->base); return;}}
#define ensure_p(x,...) \
{\
	if(!(x))\
	{\
		smoke_printf(1,__VA_ARGS__);\
\
		sockaddr_in sock_addr;\
		int namelen = sizeof(sock_addr);\
		uv_udp_getsockname(sock, (struct sockaddr *)&sock_addr, &namelen);\
		char to_addr_str[100];\
		strcpy(to_addr_str,inet_ntoa(sock_addr.sin_addr));\
		FILE * f = fopen("strange_packets.txt","a");\
		time_t t = time(NULL);\
		struct tm *tm = localtime(&t);\
		fprintf(f,"<%s> strange packet arrived from %s to %s:\n",asctime(tm),inet_ntoa(((sockaddr_in*)addr)->sin_addr),to_addr_str);\
		for(int i = 0;i < nread;i++)\
		{\
			if(i % 20 == 0) fprintf(f,"\n");\
			fprintf(f,"%02hhX",buf->base[i]);\
		}\
		fprintf(f,"\n\n");\
		for(int i = 0;i < nread;i++)\
		{\
			if(buf->base[i] > 31 && buf->base[i] < 127) fprintf(f,"%c",buf->base[i]);\
			else fprintf(f,".");\
		}\
		fprintf(f,"\n\n");\
		fclose(f);\
\
		free(buf->base);\
		return;\
	}\
}

void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
	buf->base = (char*)malloc(size);
	buf->len = size;
}

void send_cb(uv_udp_send_t* req, int status)
{
	free(req->data);
	free(req);
}

void close_and_free_cb(uv_handle_t* handle)
{
	free(handle);
}

void walk_cb(uv_handle_t* handle, void* arg)
{
	uv_close(handle, close_and_free_cb);
}

uv_timer_t* uv_timer_init_ex(uv_loop_t * loop, void * data)
{
	auto timer = (uv_timer_t*)malloc(sizeof(uv_timer_t));
	uv_timer_init(loop, timer);
	timer->data = data;
	return timer;
}

uv_timer_t* uv_timer_init_start(uv_loop_t * loop, void * data, void (*callback)(uv_timer_t*) , int timeout, int repeat)
{
	auto timer = (uv_timer_t*)malloc(sizeof(uv_timer_t));
	uv_timer_init(loop, timer);
	timer->data = data;
	uv_timer_start(timer, callback, timeout, repeat);
	return timer;
}

uv_udp_t* uv_udp_init_start(uv_loop_t *loop, void * data, void (*callback)(uv_udp_t*, ssize_t, const uv_buf_t*, const struct sockaddr*, unsigned int))
{
	auto sock = (uv_udp_t*)malloc(sizeof(uv_udp_t));
	uv_udp_init(loop, sock);
	sock->data = data;
	uv_udp_recv_start(sock, alloc_cb, callback);
	return sock;
}

uv_udp_t* uv_udp_init_start_bind(uv_loop_t *loop, void * data, void (*callback)(uv_udp_t*, ssize_t, const uv_buf_t*, const struct sockaddr*, unsigned int), const sockaddr_in* addr)
{
	auto sock = (uv_udp_t*)malloc(sizeof(uv_udp_t));
	uv_udp_init(loop, sock);
	sock->data = data;
	uv_udp_bind(sock, (const sockaddr*)addr, 0);
	uv_udp_recv_start(sock, alloc_cb, callback);
	return sock;
}

void smoke_envelope(uv_buf_t * buf, uint8_t c)
{
	buf->base[buf->len] = c;
	buf->len++;
}

unsigned char smoke_unenvelope(uv_buf_t * buf)
{
	buf->len--;
	return buf->base[buf->len];
}

void smoke_add_salt(uv_buf_t * buf, uint32_t &foam_count)
{
	uint8_t val = rand() % 256;
	for(uint32_t i=0;i<buf->len;i++)
	{
		buf->base[i] ^= val;
	}
	buf->base[buf->len] = val;
	buf->len++;

	if(buf->len > 1428) abort();
	uint16_t foam = 1428 - buf->len;
	#ifdef SMOKE_CLIENT
	foam = 0;
	#endif
	if(foam_count > 0) foam_count--; else foam = 0;
	buf->len += foam;
	(uint16_t&)buf->base[buf->len] = foam;
	buf->len += 2;
}

bool smoke_remove_salt(uv_buf_t * buf)
{
	if(buf->len < 2) return false;
	uint16_t foam = (uint16_t&)buf->base[buf->len-2];
	if(foam > buf->len-3) return false;

	buf->len -= 2;
	buf->len -= foam;

	uint8_t val = buf->base[buf->len-1];
	buf->len--;
	for(uint32_t i=0;i<buf->len;i++)
	{
		buf->base[i] ^= val;
	}
	return true;
}

bool rlnc_solve_main_matrix(int nDim, std::vector<uint8_t> &upper_matrix, std::vector<uint8_t> &lower_matrix, std::vector<uint8_t> &indexes)//TODO: redo this to use only one matrix
{
	unsigned char fMaxElem;
	unsigned char fAcc;

 	int i, j, k, m;

	for(k=0; k<nDim; k++) // base row of matrix
	{
		// search of line with max element
		fMaxElem = upper_matrix[k*nDim + k];
		m = k;
		for(i=k+1; i<nDim; i++)
		{
			if(fMaxElem < upper_matrix[i*nDim + k])
			{
				fMaxElem = upper_matrix[i*nDim + k];
				m = i;
			}
		}
    
		// permutation of base line (index k) and max element line(index m)
		if(m != k)
		{
			for(i=k; i<nDim; i++)
			{
				fAcc                     = upper_matrix[k*nDim + i];
				upper_matrix[k*nDim + i] = upper_matrix[m*nDim + i];
				upper_matrix[m*nDim + i] = fAcc;
			}
			for(i=0; i<k; i++)
			{
				fAcc                     = lower_matrix[k*nDim + i];
				lower_matrix[k*nDim + i] = lower_matrix[m*nDim + i];
				lower_matrix[m*nDim + i] = fAcc;
			}
			fAcc = indexes[k];
			indexes[k] = indexes[m];
			indexes[m] = fAcc;
		}

		if( upper_matrix[k*nDim + k] == 0) return false;

		lower_matrix[k*nDim + k] = 1;

		// triangulation of matrix with coefficients
		for(j=(k+1); j<nDim; j++) // current row of matrix
		{
			fAcc = galois_multtable_divide(upper_matrix[j*nDim + k] , upper_matrix[k*nDim + k], 8);
			lower_matrix[j*nDim + k] = fAcc;
			for(i=k; i<nDim; i++)
			{
				upper_matrix[j*nDim + i] ^= galois_multtable_multiply(fAcc,upper_matrix[k*nDim + i],8);
			}
		}
	}

	return true;
}

void rlnc_solve_lower_matrix(int nDim, std::vector<uint8_t> &lower_matrix, std::vector<uint8_t> &solutions, decrypt_generation * gen)
{
	for(int k=0; k<nDim; k++)
	{
		for(int i=0; i<k; i++)
		{
			for(int p=0;p<gen->backup_size;p++)
				solutions[k*gen->backup_size + p] ^= galois_multtable_multiply(lower_matrix[k*nDim + i],solutions[i*gen->backup_size + p],8);
		}
		for(int p=0;p<gen->backup_size;p++)
			solutions[k*gen->backup_size + p] = galois_multtable_divide(solutions[k*gen->backup_size + p] , lower_matrix[k*nDim + k],8);
	}
}

void rlnc_solve_upper_matrix(int nDim, std::vector<uint8_t> &upper_matrix, std::vector<uint8_t> &solutions, decrypt_generation * gen)
{
	for(int k=(nDim-1); k>=0; k--)
	{
		for(int i=(k+1); i<nDim; i++)
		{
			for(int p=0;p<gen->backup_size;p++)
				solutions[k*gen->backup_size + p] ^= galois_multtable_multiply(upper_matrix[k*nDim + i],solutions[i*gen->backup_size + p],8);
		}
		for(int p=0;p<gen->backup_size;p++)
			solutions[k*gen->backup_size + p] = galois_multtable_divide(solutions[k*gen->backup_size + p] , upper_matrix[k*nDim + k],8);
	}
}

double rank(double x, int y)
{
	double product = 1;
	for(int i = 0;i<y;i++) product *= x;
	return product;
}

double combinations(double k, double n)
{
	double product = 1;
	for(int i = 1;i<=k;i++)
	{
		product /= i;
		product *= n+1-i;
	}
	return product;
}

//calculating reliability of k-out-of-n system
//http://www.ewp.rpi.edu/hartford/~ernesto/S2008/SMRE/Papers/Kuo-Zuo-koon.pdf
int calculate_needed_backup(uint32_t generation_size, double estimated_drop_rate)
{
#ifdef SMOKE_CLIENT
return 0;
#endif
	double p = estimated_drop_rate;
	if(p>0.8) p = 0.8;
	uint32_t g = generation_size;
	double threshold = 0.002;//our chance of dropping a generation should be less than this
	uint32_t b = 0;//amount of backups that we'll need for this generation size, estimated drop rate and threshold
	double sum = 1;//total chance of dropping too much packets in a generation
	while(sum>threshold)
	{
		b++;
		if(b == 256) break;
		uint32_t n = b+g;//total amount of packets in a generation(payloads + backups)
		uint32_t k = b+1;//how much packets need to drop from a generation in order for it to be unsolvable.
		sum = 0;
		for(uint32_t m = k;m<=n;m++)
		{//calculating the chances of dropping exactly m packets out of n, for every value of m in (n>=m>=k) and then summing them all up to get final chance of dropping at least k packets
			double p_m = combinations(m,n) * rank(p,m) * rank(1.0-p,n-m);
			sum += p_m;
		}
	}
	return b;
}

std::tuple<uint32_t,uint32_t,uint32_t> rlnc_unenvelope(uv_buf_t &buf)
{
	uint8_t generation_index = buf.base[buf.len-1];
	buf.len--;
	uint32_t generation_size = (uint32_t&)buf.base[buf.len-4];
	buf.len -= 4;
	uint32_t generation_id = (uint32_t&)buf.base[buf.len-4];
	buf.len -= 4;
	return std::make_tuple(generation_id,generation_size,generation_index);
}

void rlnc_envelope(encrypt_context * ctx, uv_buf_t &buf, uint32_t generation_size, uint32_t generation_index)
{
	(uint32_t&)buf.base[buf.len] = ctx->generation_id;
	buf.len += 4;
	(uint32_t&)buf.base[buf.len] = generation_size;
	buf.len += 4;
	buf.base[buf.len] = (uint8_t)generation_index;
	buf.len += 1;
}

void decrypt_flush_cb(uv_timer_t *t)
{
	auto gen = (decrypt_generation*)t->data;
	smoke_printf(3,"dropping generation %d due to timeout\n",gen->id);
	gen->ctx->generation_queue.erase(gen->id);
}

void rlnc_decrypt_queue_add_message(decrypt_context * ctx, uv_buf_t &buf)
{
	auto values = rlnc_unenvelope(buf);
	uint32_t generation_id = std::get<0>(values);
	uint32_t generation_size = std::get<1>(values);
	uint32_t generation_index = std::get<2>(values);

	{//update recv table
		recv_chunk * recv_gen;
		auto&& recv_gen_pair = ctx->recv_table.find(generation_id);
		if(recv_gen_pair == ctx->recv_table.end())
		{
			recv_gen = &ctx->recv_table.emplace(generation_id,recv_chunk()).first->second;
			recv_gen->generation_id = generation_id;
			recv_gen->recv_count = 0;
		}
		else recv_gen = &recv_gen_pair->second;
		recv_gen->recv_count++;
		recv_gen->timestamp = uv_now(ctx->loop);
	}
//-----------------------------------------------------------------------------------------------------------------------
	decrypt_generation * the_generation = NULL;
	auto&& gen_iter = ctx->generation_queue.find(generation_id);
//-----------------------------------------------------------------------------------------------------------------------
	if(gen_iter == ctx->generation_queue.end())
	{
		the_generation = &ctx->generation_queue[generation_id];
		the_generation->main_buffer.resize(100 * 1500);
		the_generation->id = generation_id;
		the_generation->size = 0;
		the_generation->backup_size = 0;
		the_generation->ctx = ctx;
		the_generation->flush_timer = uv_timer_init_ex(ctx->loop,the_generation);
	}
	else
	{
		the_generation = &gen_iter->second;
	}
	uv_timer_start(the_generation->flush_timer,decrypt_flush_cb,/*ctx->round_trip_time*/20,0);
	if(generation_size > 0) the_generation->size = generation_size;
	the_generation->last_accepted_time = uv_now(ctx->loop);
	ctx->last_accepted_time = uv_now(ctx->loop);
//-----------------------------------------------------------------------------------------------------------------------
	if(generation_size > 0 && generation_index >= generation_size)
	{//this is a backup packet
		auto backup_index = generation_index - generation_size;
		if(the_generation->received_backups.count(backup_index) == 0)
		{
			the_generation->backup_size = buf.len;
			the_generation->backups.emplace_back();
			auto&& backup = the_generation->backups.back();
			backup.buffer.resize(the_generation->backup_size);
			memcpy(&backup.buffer[0], buf.base, the_generation->backup_size);
			backup.coefficient_index = generation_index - generation_size;
			the_generation->received_backups.insert(backup_index);
		}
	}
	else
	{//this is a payload packet
		if(the_generation->received_main_packets.count(generation_index) == 0)
		{

			memcpy(&the_generation->main_buffer[generation_index*1500], &buf.len, 2);
			memcpy(&the_generation->main_buffer[(generation_index*1500) + 2], buf.base, buf.len);
			the_generation->received_main_packets.insert(generation_index);

			auto p = std::make_pair(generation_id,generation_index);
			if(ctx->already_sent.find(p) == ctx->already_sent.end())
			{
				uint16_t p_size = *(uint16_t*)&the_generation->main_buffer[generation_index*1500];
				uv_buf_t tbuf = uv_buf_init((char*)malloc(1500),p_size);
				memcpy(tbuf.base, &the_generation->main_buffer[(generation_index*1500) + 2], p_size);
				sender_send(&ctx->sender, uv_now(ctx->loop), &tbuf);
				ctx->already_sent.insert(p);
				if(ctx->already_sent.size() > 1000) ctx->already_sent.erase(ctx->already_sent.begin());
			}
		}
	}
	rlnc_decrypt_try_to_solve(ctx,the_generation);
}

void rlnc_decrypt_try_to_solve(decrypt_context * ctx, decrypt_generation * gen)
{
	if(gen->size == 0) return;

	if(gen->received_main_packets.size() == gen->size)//we got all main packets, just flush
	{
		ctx->generation_queue.erase(gen->id);
		return;
	}
	else if(gen->received_main_packets.size() + gen->backups.size() < gen->size)//we got not all main packets, but not enough backups to decode
	{
		return;
	}

	while(gen->received_main_packets.size() + gen->backups.size() >= gen->size)
	{
		uint32_t matrix_diagonal_size = gen->size - gen->received_main_packets.size();

		std::vector<uint8_t> shuffle_indexes(matrix_diagonal_size);
		std::list<decrypt_backup*> bunch_of_backups;//we don't need to take all backups, only enough to decode
		{
			auto&& backup = gen->backups.begin();
			for(uint32_t i = 0;i<matrix_diagonal_size;i++)
			{
				bunch_of_backups.push_back(&(*backup));
				++backup;
				shuffle_indexes[i] = i;
			}
		}
		std::vector<uint32_t> missing_packet_indexes;
		for(uint32_t i = 0;i<gen->size;i++)
		{
			if(gen->received_main_packets.find(i) == gen->received_main_packets.end())
				missing_packet_indexes.emplace_back(i);
		}

		auto upper_matrix = std::vector<uint8_t>(matrix_diagonal_size * matrix_diagonal_size);
		auto lower_matrix = std::vector<uint8_t>(matrix_diagonal_size * matrix_diagonal_size);
		bzero(&lower_matrix[0],matrix_diagonal_size * matrix_diagonal_size);
		{
			auto&& backup = bunch_of_backups.begin();
			for(uint32_t j = 0;j<matrix_diagonal_size;j++)
			{
				for(uint32_t i = 0;i<matrix_diagonal_size;i++)
				{
					upper_matrix[j*matrix_diagonal_size+i] = coefficient_table[((*backup)->coefficient_index)*coefficient_table_row_size+missing_packet_indexes[i]];
				}
				++backup;
			}
		}

		bool solved = rlnc_solve_main_matrix(matrix_diagonal_size, upper_matrix, lower_matrix, shuffle_indexes);
		if(solved)
		{smoke_printf(3,"generation %d solved using %ld original packets and %ld out of %ld backups\n",gen->id,gen->received_main_packets.size(),bunch_of_backups.size(),gen->backups.size());
			std::vector<uint8_t> inverse_shuffle_indexes(matrix_diagonal_size);
			for(uint8_t i = 0;i<matrix_diagonal_size;i++)
			{
				inverse_shuffle_indexes[shuffle_indexes[i]] = i;
			}

			std::list<std::tuple<uint32_t,uint8_t*>> backup_pointers;
			auto solution_buffer = std::vector<uint8_t>(matrix_diagonal_size*gen->backup_size);
			uint32_t j = 0;
			for(auto&& backup : bunch_of_backups)
			{
				memcpy(&solution_buffer[inverse_shuffle_indexes[j]*gen->backup_size],&backup->buffer[0],gen->backup_size);
				backup_pointers.push_back(std::make_tuple(backup->coefficient_index, &solution_buffer[inverse_shuffle_indexes[j]*gen->backup_size]));
				j++;
			}
			std::list<std::tuple<uint32_t,uint8_t*,uint32_t>> packet_pointers;
			for(auto&& i : gen->received_main_packets)
				packet_pointers.push_back(std::make_tuple(i, &gen->main_buffer[i * 1500], 2 + *(uint16_t*)&gen->main_buffer[i * 1500]));

			rlnc_encode_backup(packet_pointers, backup_pointers);

			rlnc_solve_lower_matrix(matrix_diagonal_size, lower_matrix, solution_buffer, gen);
			rlnc_solve_upper_matrix(matrix_diagonal_size, upper_matrix, solution_buffer, gen);

			for(uint32_t i = 0;i<matrix_diagonal_size;i++)
			{
				auto p = std::make_pair(gen->id,missing_packet_indexes[i]);
				if(ctx->already_sent.find(p) == ctx->already_sent.end())
				{
					auto pos = i*gen->backup_size;
					auto p_size = *(uint16_t*)&solution_buffer[pos];
					pos += 2;
					uv_buf_t tbuf = uv_buf_init((char*)malloc(p_size),p_size);
					memcpy(tbuf.base,&solution_buffer[pos],p_size);
					sender_send(&ctx->sender,uv_now(ctx->loop),&tbuf);
					ctx->already_sent.insert(p);
					if(ctx->already_sent.size() > 1000) ctx->already_sent.erase(ctx->already_sent.begin());
				}
			}

			ctx->generation_queue.erase(gen->id);
			break;
		}
		else//if we can't solve matrix using current bunch_of_backups, it means that we can remove any one(but only one) of those backups without hurting anything
		{
			gen->backups.pop_front();
		}
	}
}

void rlnc_partial_encode_backup(uint8_t * dest, uint8_t * src, uint32_t count, uint8_t coefficient)
{
	for(uint32_t i = 0;i<count;i++)
	{
		dest[i] ^= galois_multtable_multiply(src[i], coefficient, 8);
	}
}

void rlnc_encode_backup(std::list<std::tuple<uint32_t,uint8_t*,uint32_t>> packets, std::list<std::tuple<uint32_t,uint8_t*>> backup_packets)
{
	for(auto&& backup_packet : backup_packets)
	{
		uint32_t backup_index = std::get<0>(backup_packet);
		uint8_t* backup_pointer = std::get<1>(backup_packet);
		for(auto&& packet : packets)
		{
			uint32_t packet_index = std::get<0>(packet);
			uint8_t* packet_pointer = std::get<1>(packet);
			uint32_t packet_size = std::get<2>(packet);

			uint8_t coefficient = coefficient_table[(backup_index*coefficient_table_row_size)+packet_index];
			rlnc_partial_encode_backup(backup_pointer,packet_pointer,packet_size,coefficient);
		}
	}
}

void sender_send(sender_context * sender, uint64_t time, uv_buf_t * buf)
{
	uv_udp_send_t* req = (uv_udp_send_t*)malloc(sizeof(uv_udp_send_t));
	req->data = buf->base;
	uv_udp_send(req,sender->socket,buf,1,(const sockaddr*)&sender->addr,send_cb);
	sender->last_outgoing_time = time;
}

void encrypt_flush_cb(uv_timer_t *t)
{
	rlnc_encrypt_queue_flush((encrypt_context*)t->data);
}

void rlnc_encrypt_queue_add_message(encrypt_context * ctx, uv_buf_t &buf)
{
	if(ctx->first_packet_of_generation)
	{
		ctx->first_packet_of_generation = false;
		uv_timer_start(ctx->flush_timer,encrypt_flush_cb,/*ctx->round_trip_time/10*/5,0);
	}

	if(buf.len > ctx->generation_biggest_packet_size) ctx->generation_biggest_packet_size = buf.len;

	uv_buf_t tbuf = uv_buf_init((char*)malloc(1500),buf.len);
	memcpy(tbuf.base, buf.base, buf.len);
	rlnc_envelope(ctx,tbuf,0,ctx->original_packets.size());
	smoke_envelope(&tbuf,1);
	smoke_add_salt(&tbuf,ctx->foam_count);

	sender_send(&ctx->sender, uv_now(ctx->loop), &tbuf);

	{//update send table
		send_chunk * send_gen;
		auto&& send_gen_pair = ctx->send_table.find(ctx->generation_id);
		if(send_gen_pair == ctx->send_table.end())
		{
			send_gen = &ctx->send_table.emplace(ctx->generation_id,send_chunk()).first->second;
			send_gen->generation_id = ctx->generation_id;
			send_gen->send_count = 0;
		}
		else send_gen = &send_gen_pair->second;
		send_gen->send_count++;
		send_gen->timestamp = uv_now(ctx->loop);
	}

	ctx->original_packets.emplace_back();
	auto&& packet = ctx->original_packets.back();
	packet.resize(buf.len+2);
	memcpy(&packet[0],&buf.len,2);
	memcpy(&packet[2],buf.base,buf.len);

	std::list<std::tuple<uint32_t,uint8_t*,uint32_t>> new_packet{std::make_tuple(ctx->original_packets.size()-1,&packet[0],packet.size())};

	std::list<std::tuple<uint32_t,uint8_t*>> all_backups;
	uint32_t j = 0;
	for(auto&& backup : ctx->backup_packets)
	{
		all_backups.push_back(std::make_tuple(j, &backup[0]));
		j++;
	}

	rlnc_encode_backup(new_packet, all_backups);

	uint32_t needed_backup = calculate_needed_backup(ctx->original_packets.size(), ctx->estimated_drop_rate);
	if(ctx->backup_packets.size() < needed_backup)
	{
		uint32_t old_size = ctx->backup_packets.size();
		ctx->backup_packets.resize(needed_backup);

		std::list<std::tuple<uint32_t,uint8_t*,uint32_t>> all_packets;

		uint32_t i = 0;
		for(auto&& packet : ctx->original_packets)
		{
			all_packets.push_back(std::make_tuple(i, &packet[0], packet.size()));
			i++;
		}

		std::list<std::tuple<uint32_t,uint8_t*>> new_backups;
		uint32_t z = old_size;
		for(auto&& backup = std::next(ctx->backup_packets.begin(),old_size); backup!= ctx->backup_packets.end();)
		{
			backup->resize(1500,0);
			new_backups.push_back(std::make_tuple(z, &(*backup)[0]));
			z++;
			++backup;
		}

		rlnc_encode_backup(all_packets, new_backups);
	}

	if(ctx->original_packets.size() >= ctx->max_generation_size)
	{
		uv_timer_stop(ctx->flush_timer);
		rlnc_encrypt_queue_flush(ctx);
	}
}

void rlnc_encrypt_queue_flush(encrypt_context * ctx)
{
	if(ctx->original_packets.size() == 0) return;

	uint32_t generation_index = ctx->original_packets.size();
#ifdef SMOKE_SERVER
	for(auto&& backup : ctx->backup_packets)
	{
		uv_buf_t buf = uv_buf_init((char*)malloc(1500),ctx->generation_biggest_packet_size+2);
		memcpy(buf.base, &backup[0],ctx->generation_biggest_packet_size+2);
		rlnc_envelope(ctx,buf,ctx->original_packets.size(),generation_index);
		smoke_envelope(&buf,1);
		smoke_add_salt(&buf,ctx->foam_count);
		generation_index++;
		sender_send(&ctx->sender, uv_now(ctx->loop), &buf);

		{//update send table
			send_chunk * send_gen;
			auto&& send_gen_pair = ctx->send_table.find(ctx->generation_id);
			if(send_gen_pair == ctx->send_table.end())
			{
				send_gen = &ctx->send_table.emplace(ctx->generation_id,send_chunk()).first->second;
				send_gen->generation_id = ctx->generation_id;
				send_gen->send_count = 0;
			}
			else send_gen = &send_gen_pair->second;
			send_gen->send_count++;
			send_gen->timestamp = uv_now(ctx->loop);
		}
	}
#endif
	ctx->backup_packets.clear();
	ctx->original_packets.clear();
	ctx->generation_id++;
	ctx->generation_biggest_packet_size = 0;
	ctx->first_packet_of_generation = true;
}

#ifdef SMOKE_SERVER

std::list<std::string> get_eth_addrs()
{
	std::list<std::string> result;
	struct ifaddrs *ifaddr, *ifa;
	char host[NI_MAXHOST];

	smoke_printf(1,"finding all ethernet interfaces connected to this server:\n");

	if (getifaddrs(&ifaddr) == -1)
	{
		smoke_printf(1,"getifaddrs failed\n");
		return result;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL) continue;

		int family = ifa->ifa_addr->sa_family;

		if (family == AF_INET && memcmp(ifa->ifa_name, "eth", 3) == 0)
		{
			int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0)
			{
				smoke_printf(1,"getnameinfo() failed: %s\n", gai_strerror(s));
				continue;
			}

			smoke_printf(1,"name:<%s> address: <%s>\n", ifa->ifa_name, host);
			result.emplace_back(host);
		}
	}

	freeifaddrs(ifaddr);
	return result;
}

client_info* smoke::server_init_client(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{
	ctx.clients.emplace_back();
	auto&& client = ctx.clients.back();

	client.decrypt_ctx.loop = ctx.loop;
	client.decrypt_ctx.sender.socket = uv_udp_init_start(ctx.loop, this, server_event_message_from_ovpn_server_cb);
	client.decrypt_ctx.sender.addr = ctx.ovpn_server_addr;
	client.decrypt_ctx.last_accepted_time = 0;

	client.encrypt_ctx.loop = ctx.loop;
	client.encrypt_ctx.sender.socket = sock;
	client.encrypt_ctx.sender.addr = *(sockaddr_in*)addr;

	client.encrypt_ctx.max_generation_size = 100;
	client.encrypt_ctx.generation_id = 0;
	client.encrypt_ctx.estimated_drop_rate = 0.05;
	client.encrypt_ctx.generation_biggest_packet_size = 0;
	client.encrypt_ctx.first_packet_of_generation = true;
	client.encrypt_ctx.round_trip_time = 100;
	client.decrypt_ctx.round_trip_time = 100;
	client.encrypt_ctx.foam_count = 0;
	client.encrypt_ctx.flush_timer = uv_timer_init_ex(sock->loop,&client.encrypt_ctx);

	client.session_key = std::vector<uint8_t>(256);
	for(int i = 0;i<client.session_key.size();i++)
		client.session_key[i] = rand() % 256;

	return &client;
}

void server_event_message_from_client_cb(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{
	smoke* that = (smoke*)sock->data;
	that->server_event_message_from_client(sock, nread, buf, addr, flags);
}

void smoke::server_event_message_from_client(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{//printf("server_event_message_from_client start\n");
	ensure(nread !=0);
	ensure_p(nread > 0,"network error %ld has occured while receiving packet\n",nread);
	ensure_p(nread <= 1500, "packet received from client is too big(%ld), dropping\n",nread);

	uv_buf_t tbuf = uv_buf_init(buf->base, nread);
	ensure_p(smoke_remove_salt(&tbuf),"packet received from client has wrong salt, dropping\n");
	uint8_t packet_type = smoke_unenvelope(&tbuf);
	
	client_info * the_client = NULL;
	for(auto&& client : ctx.clients)
	{
		if(memcmp(&client.encrypt_ctx.sender.addr, addr,sizeof(sockaddr_in))==0)
		{
			the_client = &client;
			break;
		}
		else if(memcmp(&client.ip_hop_addr, addr,sizeof(sockaddr_in))==0)
		{
			the_client = &client;
			the_client->encrypt_ctx.sender.socket = sock;
			the_client->encrypt_ctx.sender.addr = *(sockaddr_in*)addr;
			the_client->encrypt_ctx.foam_count = the_client->big_packets_at_start;
			break;
		}
	}

	if(packet_type == 0 && the_client != NULL)
	{
		//this is service packet, it contains drop rate
		ensure_p(tbuf.len == 20, "packet received from server is of type 0, but it's size is wrong, dropping(expected size = 20, received size = %ld)\n",tbuf.len);//4+4+4+8 = 20
		uint32_t first_gen = *(uint32_t*)&tbuf.base[0];
		uint32_t last_gen  = *(uint32_t*)&tbuf.base[4];
		int32_t received  = *(int32_t*)&tbuf.base[8];
		uint64_t timestamp  = *(uint64_t*)&tbuf.base[12];
		if(received >= 0)
		{	
			int sent = 0;
			for(auto&& send_instance = the_client->encrypt_ctx.send_table.begin(); send_instance != the_client->encrypt_ctx.send_table.end();)
			{
				if(send_instance->second.generation_id >= first_gen && send_instance->second.generation_id <= last_gen)
					sent += send_instance->second.send_count;

				if(uv_now(ctx.loop) - send_instance->second.timestamp >= 15000)
					send_instance = the_client->encrypt_ctx.send_table.erase(send_instance);
				else
					++send_instance;
			}

			if(sent > 0) the_client->encrypt_ctx.estimated_drop_rate = 1.0 - (double)received / sent;
			smoke_printf(2,"outgoing drop rate = %f, sent = %d, received = %d from g%d to g%d\n",the_client->encrypt_ctx.estimated_drop_rate,sent,received,first_gen,last_gen);
		}

		{//send echo back
			uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),8);
			*(uint64_t*)&send_buf.base[0] = timestamp;
			smoke_envelope(&send_buf,2);
			smoke_add_salt(&send_buf,the_client->encrypt_ctx.foam_count);
			sender_send(&the_client->encrypt_ctx.sender, uv_now(sock->loop), &send_buf);
		}
	}
	else if(packet_type == 1 && the_client != NULL)
	{
		//this is a network-coded message
		ensure_p(tbuf.len >= 9, "packet received from server is of type 1, but it's size is wrong, dropping(expected size >= 9, received size = %ld)\n",tbuf.len);//4+4+1 = 9
		rlnc_decrypt_queue_add_message(&the_client->decrypt_ctx, tbuf);
	}
	else if(packet_type == 2 && the_client != NULL)
	{
		//this is an echo packet that only contains timestamp of original send on our side
		ensure_p(tbuf.len == 8, "packet received from server is of type 2, but it's size is wrong, dropping(expected size = 8, received size = %ld)\n",tbuf.len);
		uint64_t timestamp  = *(uint64_t*)&tbuf.base[0];
		uint64_t RTT = uv_now(sock->loop) - timestamp;
		the_client->encrypt_ctx.round_trip_time = RTT;
		the_client->decrypt_ctx.round_trip_time = RTT;
		smoke_printf(2,"latency = %lu ms\n",RTT);
	}
	else if(packet_type == 3)
	{
		//this is handshake(client->server) packet, it contains version and communication options
		ensure_p(tbuf.len == 8, "packet received from server is of type 3, but it's size is wrong, dropping(expected size = 8, received size = %ld)\n",tbuf.len);//4+4 = 8
		uint32_t client_version =    *(uint32_t*)&tbuf.base[0];
		uint32_t big_packet_amount = *(uint32_t*)&tbuf.base[4];
		if(client_version <= SMOKE_PROTOCOL_VERSION_TOO_SMALL || client_version >= SMOKE_PROTOCOL_VERSION_TOO_BIG)
		{
			sender_context tmp_sender;
			uint32_t tmp_int = big_packet_amount;
			tmp_sender.socket = sock;
			tmp_sender.addr = *(sockaddr_in*)addr;
			uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),4);
			*(uint32_t*)&send_buf.base[0] = 0;
			smoke_envelope(&send_buf,4);
			smoke_add_salt(&send_buf,tmp_int);
			sender_send(&tmp_sender, uv_now(sock->loop), &send_buf);
			smoke_printf(1,"rejecting handshake(wrong version = %d, %d)\n",client_version,big_packet_amount);
		}
		else
		{
			if(the_client == NULL)
				the_client = server_init_client(sock, nread, buf, addr, flags);

			the_client->client_version = client_version;
			the_client->big_packets_at_start = big_packet_amount;
			the_client->encrypt_ctx.foam_count = big_packet_amount;

			uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),4+the_client->session_key.size());
			*(uint32_t*)&send_buf.base[0] = 1;
			memcpy(&send_buf.base[4],&the_client->session_key[0],the_client->session_key.size());
			smoke_envelope(&send_buf,4);
			smoke_add_salt(&send_buf,the_client->encrypt_ctx.foam_count);
			sender_send(&the_client->encrypt_ctx.sender, uv_now(sock->loop), &send_buf);
			smoke_printf(1,"accepting handshake\n");
		}
	}
	else if(packet_type == 5)
	{
		//this is ip hop(client->server) packet, it contains old client session_key
		std::vector<uint8_t> session_key(tbuf.base,tbuf.base+tbuf.len);
		for(auto&& client : ctx.clients)
		{
			if(client.session_key == session_key)
			{
				the_client = &client;
				break;
			}
		}
		if(the_client!=NULL)
		{
			the_client->ip_hop_addr = *(sockaddr_in*)addr;

			sender_context tmp_sender;
			uint32_t tmp_int = the_client->big_packets_at_start;
			tmp_sender.socket = sock;
			tmp_sender.addr = *(sockaddr_in*)addr;

			uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),4);
			*(uint32_t*)&send_buf.base[0] = 1;
			smoke_envelope(&send_buf,6);
			smoke_add_salt(&send_buf,tmp_int);
			sender_send(&tmp_sender, uv_now(sock->loop), &send_buf);
			smoke_printf(1,"accepting ip hop\n");
		}
		else
		{
			sender_context tmp_sender;
			uint32_t tmp_int = 1;
			tmp_sender.socket = sock;
			tmp_sender.addr = *(sockaddr_in*)addr;

			uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),4);
			*(uint32_t*)&send_buf.base[0] = 0;
			smoke_envelope(&send_buf,6);
			smoke_add_salt(&send_buf,tmp_int);
			sender_send(&tmp_sender, uv_now(sock->loop), &send_buf);
			smoke_printf(1,"rejecting ip hop\n");
		}
	}

	if(the_client!=NULL)
		the_client->last_incoming_time = uv_now(sock->loop);

	free(buf->base);
//printf("server_event_message_from_client end\n");
}

void server_event_message_from_ovpn_server_cb(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{
	smoke* that = (smoke*)sock->data;
	that->server_event_message_from_ovpn_server(sock, nread, buf, addr, flags);
}

void smoke::server_event_message_from_ovpn_server(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags)
{//printf("server_event_message_from_server start\n");
	ensure(nread !=0);
	ensure_p(nread > 0,"network error %ld has occured while receiving packet\n",nread);
	ensure_p(nread <= 1400, "packet received from server is too big(%ld), dropping\n",nread);

	uv_buf_t tbuf = uv_buf_init(buf->base, nread);

	client_info * the_client = NULL;
	for(auto&& client : ctx.clients)
	{
		if(client.decrypt_ctx.sender.socket == sock)
		{
			the_client = &client;
			break;
		}
	}

	if(the_client != NULL)
		rlnc_encrypt_queue_add_message(&the_client->encrypt_ctx, tbuf);

	free(buf->base);
//printf("server_event_message_from_server end\n");
}

void server_event_every_second_cb(uv_timer_t *t)
{
	smoke* that = (smoke*)t->data;
	that->server_event_every_second(t);
}

void smoke::server_event_every_second(uv_timer_t *t)
{//printf("server_event_every_second start\n");
	for(auto&& client = ctx.clients.begin();client != ctx.clients.end();)
	{
		if(uv_now(t->loop) - client->last_incoming_time >= 60000)
		{
			uv_close((uv_handle_t*)client->decrypt_ctx.sender.socket,close_and_free_cb);
			uv_close((uv_handle_t*)client->encrypt_ctx.flush_timer,close_and_free_cb);
			client = ctx.clients.erase(client);
		}
		else
		{
			uv_buf_t buf = uv_buf_init((char*)malloc(1500), 0);

			if(client->decrypt_ctx.recv_table.size()>1)
			{
				auto&& first_inst = client->decrypt_ctx.recv_table.begin();
				auto&& last_inst = std::prev(client->decrypt_ctx.recv_table.end());
				auto&& almost_last_inst = std::prev(last_inst);
				buf.len += sizeof(uint32_t)*3 + sizeof(uint64_t);
				*(uint32_t*)&buf.base[0] = first_inst->second.generation_id;
				*(uint32_t*)&buf.base[4] = almost_last_inst->second.generation_id;

				int32_t recv_packet_count = 0;

				for (auto&& recv_instance = first_inst; recv_instance != last_inst;)
				{
					recv_packet_count += recv_instance->second.recv_count;
					if (uv_now(t->loop) - recv_instance->second.timestamp >= 10000)
					    recv_instance = client->decrypt_ctx.recv_table.erase(recv_instance);
					else
					    ++recv_instance;
				}

				*(int32_t*)&buf.base[8] = recv_packet_count;
				*(uint64_t*)&buf.base[12] = uv_now(t->loop);
			}
			else
			{
				*(uint32_t*)&buf.base[0] = 0;
				*(uint32_t*)&buf.base[4] = 0;
				*(int32_t*)&buf.base[8] = -1;
				*(uint64_t*)&buf.base[12] = uv_now(t->loop);
				buf.len += sizeof(uint32_t)*3 + sizeof(uint64_t);
			}

			smoke_envelope(&buf,0);
			smoke_add_salt(&buf,client->encrypt_ctx.foam_count);
			sender_send(&client->encrypt_ctx.sender,uv_now(t->loop),&buf);

			++client;
		}
	}
//printf("server_event_every_second end\n");
}

void smoke::start_server(int local_udp_port, int ovpn_server_udp_port, std::string ovpn_server_ip = "127.0.0.1")
{
	//init_variables
	auto&& eth_addrs = get_eth_addrs();
	if(eth_addrs.size() == 0)
	{
		smoke_printf(1,"didn't find any ip addresses to listen on, exiting\n");
		return;
	}
	srand(time(0));
	galois_create_mult_tables(8);
	uv_loop_t loop;
	uv_loop_init(&loop);
	ctx.loop = &loop;
	//open_ports_and_sockets
	for(auto&& eth_addr : eth_addrs)
	{
		ctx.client_sockets.emplace_back();
		auto&& client_socket = ctx.client_sockets.back();
		uv_ip4_addr(eth_addr.c_str(), local_udp_port, &client_socket.addr);
		client_socket.socket = uv_udp_init_start_bind(ctx.loop, this, server_event_message_from_client_cb, &client_socket.addr);
	}

	uv_ip4_addr(ovpn_server_ip.c_str(), ovpn_server_udp_port, &ctx.ovpn_server_addr);

	//register_events
	ctx.per_second_handler = uv_timer_init_start(ctx.loop, this, server_event_every_second_cb, 0, 1000);

	//start_loop
	uv_run(ctx.loop, UV_RUN_DEFAULT);

	//free
	uv_walk(ctx.loop, walk_cb, NULL);

	uv_run(ctx.loop, UV_RUN_DEFAULT);

	ctx.client_sockets.clear();
	ctx.clients.clear();

	uv_loop_close(&loop);
}
#elif SMOKE_CLIENT
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void connection_timeout_handler_cb(uv_timer_t *t)
{
	smoke* that = (smoke*)t->data;
	that->connection_timeout_handler(t);
}

void smoke::connection_timeout_handler(uv_timer_t *t)
{printf("hop from connection_timeout_handler\n");
	ctx.connection_state = connection_lost;
	ctx.hops_left = hop_attempts;
	client_request_hop();
}


void regular_hop_handler_cb(uv_timer_t *t)
{
	smoke* that = (smoke*)t->data;
	that->regular_hop_handler(t);
}

void smoke::regular_hop_handler(uv_timer_t *t)
{printf("hop from regular_hop_handler\n");
	ctx.hops_left = hop_attempts;
	client_request_hop();
}


void hop_timeout_handler_cb(uv_timer_t *t)
{
	smoke* that = (smoke*)t->data;
	that->hop_timeout_handler(t);
}

void smoke::hop_timeout_handler(uv_timer_t *t)
{printf("hop from hop_timeout_handler\n");
	client_hop_failed();
}

void client_event_message_from_client_cb(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, uint32_t flags)
{
	smoke* that = (smoke*)sock->data;
	that->client_event_message_from_client(sock, nread, buf, addr, flags);
}

void smoke::client_event_message_from_client(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, uint32_t flags)
{
	//printf("client_event_message_from_client start\n");
	ensure(nread !=0);
	ensure_p(nread > 0,"network error %ld has occured while receiving packet\n",nread);
	ensure_p(nread <= 1400, "packet received from client is too big(%ld), dropping\n",nread);
	ensure(ctx.connection_state == connection_established);

	uv_buf_t tbuf = uv_buf_init(buf->base, nread);
	ctx.connection.decrypt_ctx.sender.addr = *(sockaddr_in*)addr;
	rlnc_encrypt_queue_add_message(&ctx.connection.encrypt_ctx, tbuf);

	free(buf->base);
	//printf("client_event_message_from_client end\n");
}

void client_event_message_from_server_cb(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, uint32_t flags)
{
    smoke* that = (smoke*)sock->data;
    that->client_event_message_from_server(sock, nread, buf, addr, flags);
}

void smoke::client_event_message_from_server(uv_udp_t *sock, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, uint32_t flags)
{
	//printf("client_event_message_from_server start %ld\n",nread);
	ensure(nread !=0);
	ensure_p(nread > 0,"network error %ld has occured while receiving packet\n",nread);
	ensure_p(nread <= 1500, "packet received from server is too big(%ld), dropping\n",nread);

	uv_buf_t tbuf = uv_buf_init(buf->base, nread);

	ctx.connection.last_incoming_time = uv_now(sock->loop);

	ensure_p(smoke_remove_salt(&tbuf),"packet received from server has wrong salt, dropping\n");
	uint8_t packet_type = smoke_unenvelope(&tbuf);

	if(packet_type == 0)
	{
		//this is service packet, it contains drop rate
		ensure_p(tbuf.len == 20, "packet received from server is of type 0, but it's size is wrong, dropping(expected size = 20, received size = %ld)\n",tbuf.len);//4+4+4+8 = 20
		uint32_t first_gen = *(uint32_t*)&tbuf.base[0];
		uint32_t last_gen  = *(uint32_t*)&tbuf.base[4];
		int32_t received  = *(int32_t*)&tbuf.base[8];
		uint64_t timestamp = *(uint64_t*)&tbuf.base[12];
		if(received >= 0)
		{
			int sent  = 0;
			for(auto&& send_instance = ctx.connection.encrypt_ctx.send_table.begin(); send_instance != ctx.connection.encrypt_ctx.send_table.end();)
			{
				if(send_instance->second.generation_id >= first_gen && send_instance->second.generation_id <= last_gen)
					sent += send_instance->second.send_count;

				if(uv_now(ctx.loop) - send_instance->second.timestamp >= 15000)
					send_instance = ctx.connection.encrypt_ctx.send_table.erase(send_instance);
				else
					++send_instance;
			}

			if(sent > 0) ctx.connection.encrypt_ctx.estimated_drop_rate = 1.0 - (double)received / sent;
			smoke_printf(2,"outgoing drop rate = %f, sent = %d, received = %d from g%d to g%d\n",ctx.connection.encrypt_ctx.estimated_drop_rate, sent, received,first_gen,last_gen);
		}

		{//send echo back
			uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),8);
			*(uint64_t*)&send_buf.base[0] = timestamp;
			smoke_envelope(&send_buf,2);
			smoke_add_salt(&send_buf,ctx.connection.encrypt_ctx.foam_count);
			sender_send(&ctx.connection.encrypt_ctx.sender, uv_now(sock->loop), &send_buf);
		}
	}
	else if(packet_type == 1)
	{
		//this is a network-coded message
		ensure_p(tbuf.len >= 9, "packet received from server is of type 1, but it's size is wrong, dropping(expected size = 9, received size = %ld)\n",tbuf.len);
		rlnc_decrypt_queue_add_message(&ctx.connection.decrypt_ctx, tbuf);
	}
	else if(packet_type == 2)
	{
		//this is an echo packet that only contains timestamp of original send on our side
		ensure_p(tbuf.len == 8, "packet received from server is of type 2, but it's size is wrong, dropping(expected size = 8, received size = %ld)\n",tbuf.len);
		uint64_t timestamp  = *(uint64_t*)&tbuf.base[0];
		uint64_t RTT = uv_now(sock->loop) - timestamp;
		ctx.connection.encrypt_ctx.round_trip_time = RTT;
		ctx.connection.decrypt_ctx.round_trip_time = RTT;
		smoke_printf(2,"latency = %lu ms\n",RTT);
	}
	else if(packet_type == 4)
	{
		//this is handshake response(server->client) packet, it contains handshake result and session key(if successful)
		ensure_p(tbuf.len >= 4, "packet received from server is of type 4, but it's size is wrong, dropping(expected size >= 4, received size = %ld)\n",tbuf.len);
		uint32_t connection_ok  = *(uint32_t*)&tbuf.base[0];
		if(connection_ok == 1)
		{
			size_t size = tbuf.len - sizeof(connection_ok);
			if(size>0)
			{
				ctx.connection.session_key = std::vector<uint8_t>(size);
				memcpy(&ctx.connection.session_key[0],&tbuf.base[4],size);
			}
			ctx.connection_state = connection_established;
			smoke_printf(1,"connection established\n");
			uv_timer_start(ctx.regular_hop_handler, regular_hop_handler_cb, hop_interval, 0);
			uv_timer_stop(ctx.handshake_timeout_handler);
			#ifdef SMOKE_QT
			emit connection_established(ntohs(ctx.local_addr.sin_port));
			#endif
		}
		else
		{
			smoke_printf(1,"connection refused by server, exiting\n");
			uv_stop(ctx.loop);
			#ifdef SMOKE_QT
			emit connection_refused();
			#endif
		}
	}
	else if(packet_type == 6)
	{
		//this is ip hop response(server->client) packet
		ensure_p(tbuf.len == 4, "packet received from server is of type 6, but it's size is wrong, dropping(expected size = 4, received size = %ld)\n",tbuf.len);
		uint32_t hop_ok  = *(uint32_t*)&tbuf.base[0];
		if(hop_ok == 1)
		{
			smoke_printf(1,"connection established after hop\n");

			ctx.connection_state = connection_established;
			uv_timer_start(ctx.regular_hop_handler, regular_hop_handler_cb, hop_interval, 0);
			uv_timer_stop(ctx.hop_timeout_handler);

			uv_close((uv_handle_t*)ctx.connection.encrypt_ctx.sender.socket,close_and_free_cb);
			ctx.connection.encrypt_ctx.sender.socket = ctx.hop_ctx.socket;
			ctx.connection.encrypt_ctx.sender.addr = ctx.hop_ctx.addr;
			ctx.hop_ctx.socket = NULL;
		}
		else
		{
			printf("hop from failed hop response\n");
			uv_timer_stop(ctx.hop_timeout_handler);
			client_hop_failed();
		}
	}

	if(ctx.connection_state == connection_established) uv_timer_start(ctx.connection_timeout_handler, connection_timeout_handler_cb, connection_timeout, 0);

	free(buf->base);
	//printf("client_event_message_from_server end\n");
}

void client_handshake_timeout_cb(uv_timer_t *t)
{
	smoke* that = (smoke*)t->data;
	that->client_handshake_timeout(t);
}

void smoke::client_handshake_timeout(uv_timer_t *t)
{
	smoke_printf(1,"handshake timeout, exiting\n");
	uv_stop(ctx.loop);
	#ifdef SMOKE_QT
	emit connection_timeout();
	#endif
}

void client_event_every_second_cb(uv_timer_t *t)
{
	smoke* that = (smoke*)t->data;
	that->client_event_every_second(t);
}

void smoke::client_event_every_second(uv_timer_t *t)
{
	//printf("client_event_every_second start\n");

	uv_buf_t buf = uv_buf_init((char*)malloc(1500), 0);

	if(ctx.connection_state == connection_established)
	{
		if(ctx.connection.decrypt_ctx.recv_table.size()>1)
		{
			auto&& first_inst = ctx.connection.decrypt_ctx.recv_table.begin();
			auto&& last_inst = std::prev(ctx.connection.decrypt_ctx.recv_table.end());
			auto&& almost_last_inst = std::prev(last_inst);
			buf.len += sizeof(uint32_t)*3 + sizeof(uint64_t);
			*(uint32_t*)&buf.base[0] = first_inst->second.generation_id;
			*(uint32_t*)&buf.base[4] = almost_last_inst->second.generation_id;

			int32_t recv_packet_count = 0;

			for (auto&& recv_instance = first_inst; recv_instance != last_inst;)
			{
				recv_packet_count += recv_instance->second.recv_count;
				if (uv_now(t->loop) - recv_instance->second.timestamp >= 10000)
					recv_instance = ctx.connection.decrypt_ctx.recv_table.erase(recv_instance);
				else
					++recv_instance;
			}

			*(int32_t*)&buf.base[8] = recv_packet_count;
			*(uint64_t*)&buf.base[12] = uv_now(t->loop);
		}
		else
		{
			*(uint32_t*)&buf.base[0] = 0;
			*(uint32_t*)&buf.base[4] = 0;
			*(int32_t*)&buf.base[8] = -1;
			*(uint64_t*)&buf.base[12] = uv_now(t->loop);
			buf.len += sizeof(uint32_t)*3 + sizeof(uint64_t);
		}

		smoke_envelope(&buf,0);
		smoke_add_salt(&buf,ctx.connection.encrypt_ctx.foam_count);
		sender_send(&ctx.connection.encrypt_ctx.sender,uv_now(t->loop),&buf);
	}
	else if(ctx.connection_state == connection_not_established)
	{
		*(uint32_t*)&buf.base[0] = SMOKE_PROTOCOL_VERSION;
		*(uint32_t*)&buf.base[4] = request_big_packets;
		buf.len = 8;
		smoke_envelope(&buf,3);	
		smoke_add_salt(&buf,ctx.connection.encrypt_ctx.foam_count);
		sender_send(&ctx.connection.encrypt_ctx.sender,uv_now(t->loop),&buf);
	}
	//printf("client_event_every_second end\n");
}

void smoke::start_client(int local_udp_port, int smoke_server_udp_port, std::string smoke_server_ip = "127.0.0.1")
{
	//init_variables
	srand(time(0));
	galois_create_mult_tables(8);
	uv_loop_t loop;
	uv_loop_init(&loop);
	ctx.loop = &loop;
	//open_ports_and_sockets
	uv_ip4_addr("127.0.0.1", local_udp_port, &ctx.local_addr);
	uv_ip4_addr(smoke_server_ip.c_str(), smoke_server_udp_port, &ctx.smoke_server_addr);

	ctx.connection_state = connection_not_established;
	ctx.hops_left = 0;
	ctx.connection.decrypt_ctx.loop = ctx.loop;
	ctx.connection.decrypt_ctx.sender.socket = uv_udp_init_start_bind(ctx.loop, this, client_event_message_from_client_cb, &ctx.local_addr);
	if(local_udp_port == 0)
	{
		int namelen = sizeof(ctx.local_addr);
		uv_udp_getsockname(ctx.connection.decrypt_ctx.sender.socket, (struct sockaddr *)&ctx.local_addr, &namelen);
	}
	ctx.connection.decrypt_ctx.last_accepted_time = 0;

	ctx.connection.encrypt_ctx.loop = ctx.loop;
	ctx.connection.encrypt_ctx.sender.addr = ctx.smoke_server_addr;
	ctx.connection.encrypt_ctx.sender.socket = uv_udp_init_start(ctx.loop, this, client_event_message_from_server_cb);

	ctx.connection.encrypt_ctx.max_generation_size = 100;
	ctx.connection.encrypt_ctx.generation_id = 0;
	ctx.connection.encrypt_ctx.estimated_drop_rate = 0.05;
	ctx.connection.encrypt_ctx.generation_biggest_packet_size = 0;
	ctx.connection.encrypt_ctx.first_packet_of_generation = true;
	ctx.connection.encrypt_ctx.round_trip_time = 100;
	ctx.connection.decrypt_ctx.round_trip_time = 100;
	ctx.connection.encrypt_ctx.foam_count = 0;
	ctx.connection.encrypt_ctx.flush_timer = uv_timer_init_ex(ctx.loop, &ctx.connection.encrypt_ctx);

	ctx.hop_ctx.socket = NULL;

	//register_events

	ctx.per_second_handler = uv_timer_init_start(ctx.loop, this, client_event_every_second_cb, 0, 1000);
	ctx.handshake_timeout_handler = uv_timer_init_start(ctx.loop, this, client_handshake_timeout_cb, handshake_timeout, 0);
	ctx.regular_hop_handler = uv_timer_init_ex(ctx.loop,this);
	ctx.hop_timeout_handler = uv_timer_init_ex(ctx.loop,this);
	ctx.connection_timeout_handler = uv_timer_init_ex(ctx.loop,this);

	//start_loop
	uv_run(ctx.loop, UV_RUN_DEFAULT);

	//free
	uv_walk(ctx.loop, walk_cb, NULL);

	uv_run(ctx.loop, UV_RUN_DEFAULT);

	ctx.connection.encrypt_ctx.backup_packets.clear();
	ctx.connection.encrypt_ctx.original_packets.clear();

	ctx.connection.encrypt_ctx.send_table.clear();
	ctx.connection.decrypt_ctx.generation_queue.clear();
	ctx.connection.decrypt_ctx.recv_table.clear();

	uv_loop_close(&loop);
}

void smoke::client_request_hop()
{
	std::string smoke_server_ip = remote_hosts[remote_host_index].first;
	int remote_port = remote_hosts[remote_host_index].second;
	remote_host_index++;
	if(remote_host_index > remote_hosts.size()-1) remote_host_index = 0;
	if(ctx.hop_ctx.socket != NULL)
	{
		uv_close((uv_handle_t*)ctx.hop_ctx.socket,close_and_free_cb);
	}

	uv_ip4_addr(smoke_server_ip.c_str(), remote_port, &ctx.hop_ctx.addr);
	ctx.hop_ctx.socket = uv_udp_init_start(ctx.loop, this, client_event_message_from_server_cb);

	uv_buf_t send_buf = uv_buf_init((char*)malloc(1500),ctx.connection.session_key.size());
	memcpy(send_buf.base,&ctx.connection.session_key[0],ctx.connection.session_key.size());
	smoke_envelope(&send_buf,5);
	smoke_add_salt(&send_buf,ctx.connection.encrypt_ctx.foam_count);
	sender_send(&ctx.hop_ctx,uv_now(ctx.loop),&send_buf);
	uv_timer_start(ctx.hop_timeout_handler, hop_timeout_handler_cb, hop_timeout, 0);
}

void smoke::client_hop_failed()
{
	if(ctx.hops_left > 0)
	{
		ctx.hops_left--;
		client_request_hop();
	}
	else if(ctx.connection_state == connection_lost)
	{
		smoke_printf(1,"failed to hop after losing connection, exiting\n");
		uv_stop(ctx.loop);
		//#ifdef SMOKE_QT
		//emit connection_timeout();
		//#endif
	}
}

#endif

void smoke::smoke_main()
{
	#ifdef SMOKE_SERVER
	#define default_local_port 3758
	#define default_server_port 7772
	#define default_server_ip "127.0.0.1"
	int local_port = default_local_port;
	int server_port = default_server_port;
	char server_ip[50] = default_server_ip;
	FILE * f = fopen("smoke_server.conf","r");
	if(f != NULL)
	{
		fscanf(f,"%d %s %d",&local_port,server_ip,&server_port);
		fclose(f);
	}
	else
	{
		smoke_printf(1,"couldn't load smoke_server.conf, starting with default params\n");
	}

	smoke_printf(1,"starting server on udp port %d, sending packets to %s:%d\n",local_port,server_ip,server_port);
	start_server(local_port, server_port, server_ip);
	#elif SMOKE_CLIENT
	auto&& actual_host = remote_hosts[0];
	remote_host_index = 1;
	if(remote_host_index > remote_hosts.size()-1) remote_host_index = 0;
	if(local_port == 0)
	{
		smoke_printf(1,"starting client on first available udp port, sending packets to %s:%d\n", actual_host.first.c_str(), actual_host.second);
	}
	else
	{
		smoke_printf(1,"starting client on udp port %d, sending packets to %s:%d\n", local_port, actual_host.first.c_str(), actual_host.second);
	}
	start_client(local_port, actual_host.second, actual_host.first);
	#else
	#error compile with -DSMOKE_SERVER or -DSMOKE_CLIENT
	#endif
}

smoke::smoke()
{

}

#ifdef SMOKE_QT
void smoke::close_smoke()
{
	uv_stop(ctx.loop);
}

void smoke::run()
{
	smoke_main();
}
#else
int main()
{
	smoke that;
#ifdef SMOKE_SERVER
#elif SMOKE_CLIENT
	that.request_big_packets = 0;
	that.local_port = 4987;
	that.remote_host_index = 0;
	that.handshake_timeout = 5000;
	that.connection_timeout = 10000;
	that.hop_timeout = 5000;
	that.hop_interval = 30 * 60 * 1000;
	that.hop_attempts = 10;
	char first[256];
	char rest[256];
	FILE * f = fopen("smoke_client.conf","r");
	if(f != NULL)
	{
		while(!feof(f))
		{
			fscanf(f,"%s %s",first,rest);
			if(strcmp(first,"remote")==0)
			{
				char ip[256];
				int port;
				int i = 0;
				while(rest[i]!=':' && rest[i]!='\0') i++;
				if(rest[i] == '\0')
				{
					printf("no : symbol found in remote entry, skipping\n");
					continue;
				}
				rest[i] = ' ';
				sscanf(rest,"%s %d",ip,&port);
				that.remote_hosts.emplace_back(std::make_pair(ip,port));
			}
			else
			{
				int d;
				sscanf(rest,"%d",&d);
				if(strcmp(first,"local_port")==0) that.local_port = d;
				else if(strcmp(first,"hop_interval_seconds")==0) that.hop_interval = d * 1000;
				else if(strcmp(first,"hop_interval_minutes")==0) that.hop_interval = d * 1000 * 60;
				else if(strcmp(first,"hop_interval_hours")==0) that.hop_interval = d * 1000 * 360;
				else if(strcmp(first,"hop_timeout_seconds")==0) that.hop_timeout = d * 1000;
				else if(strcmp(first,"hop_timeout_minutes")==0) that.hop_timeout = d * 1000 * 60;
				else if(strcmp(first,"hop_timeout_hours")==0) that.hop_timeout = d * 1000 * 360;
				else if(strcmp(first,"connection_timeout_seconds")==0) that.connection_timeout = d * 1000;
				else if(strcmp(first,"connection_timeout_minutes")==0) that.connection_timeout = d * 1000 * 60;
				else if(strcmp(first,"connection_timeout_hours")==0) that.connection_timeout = d * 1000 * 360;
				else if(strcmp(first,"handshake_timeout_seconds")==0) that.handshake_timeout = d * 1000;
				else if(strcmp(first,"handshake_timeout_minutes")==0) that.handshake_timeout = d * 1000 * 60;
				else if(strcmp(first,"handshake_timeout_hours")==0) that.handshake_timeout = d * 1000 * 360;
				else if(strcmp(first,"request_big_packets")==0) that.request_big_packets = d;
				else if(strcmp(first,"hop_attempts")==0) that.hop_attempts = d;
			}
		}
		fclose(f);
	}
	else
	{
		printf("couldn't load smoke_server.conf, starting with default params\n");
		that.remote_hosts = {std::make_pair("45.32.181.159",8080)};
	}
#endif
	that.smoke_main();
	return 0;
}
#endif
