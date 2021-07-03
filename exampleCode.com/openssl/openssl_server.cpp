#include <iostream>
#include <openssl/ssl.h>

int main(int argc, char* argv[])
{
    char *port = "*:4433";
    BIO *ssl_bio, *tmp;
    SSL_CTX *ctx;
    SSL_CONF_CTX *cctx;
    char buf[512];

    BIO *in = nullptr;
    int ret = EXIT_FAILURE, i;
    char **args = argv + 1;
    int nargs = argc - 1;


    ctx = SSL_CTX_new(TLS_server_method());
    cctx = SSL_CONF_CTX_new();
    SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_SERVER);
    SSL_CONF_CTX_set_flags(cctx, SSL_CONF_FLAG_CERTIFICATE);
    SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);

    if (!SSL_CONF_CTX_finish(cctx)) {
        std::cout << "ssl conf ctx finish error " << std::endl;
        return 0;
    }

    ssl_bio = BIO_new_ssl(ctx, 0);
    
    if ((in = BIO_new_accept(port)) == nullptr) {
        return 0;
    }

    BIO_set_accept_bios(in, ssl_bio);

again:
    if (BIO_do_accept(in) <= 0) {
        return 0;
    }

    std::cout<< "start server " << std::endl;

    for (;;) {
        i = BIO_read(in, buf, 512);
        std::cout << "!done!!" << std::endl;

        if ( i == 0){
            printf("Done\n");
            tmp = BIO_pop(in);
            BIO_free_all(tmp);
            goto again;
        }

        if (i < 0) {
            return 0;
        }

        fwrite(buf, 1, i , stdout);
        fflush(stdout);
    }

    BIO_free(in);


    return 0;
}
