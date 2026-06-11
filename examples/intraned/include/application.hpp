#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "module_manager.hpp"
#include "i_http_server.hpp"
#include "i_database.hpp"
#include "json.hpp" // Header-only seguro
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sys/stat.h>

using json = nlohmann::json;

class Application 
{
    private:
        ModuleManager module_manager_;
        std::shared_ptr<IHttpServer> server_;
        std::shared_ptr<IDatabase> db_;

        const std::string ADMIN_TOKEN = "educacion_libre_2026";
        const std::string UPLOAD_DIR = "./uploads/";

        void asegurar_directorio(const std::string& path) 
        {
            struct stat info;
            if (stat(path.c_str(), &info) != 0) 
            {
                mkdir(path.c_str(), 0777);
            }
        }

        void configure_database()
        {
            // Migración desde JSON a esquema relacional (Sustituye a fileHelper.cpp)
            std::string ddl = "CREATE TABLE IF NOT EXISTS recursos ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "titulo TEXT NOT NULL, "
                              "autor TEXT NOT NULL, "
                              "tema TEXT NOT NULL, "
                              "filename TEXT NOT NULL);";
            db_->execute(ddl, {});
        }

        void configure_routes() 
        {
            // --- 1. ENTREGA DE ACTIVOS ESTÁTICOS (Frontend) ---
            server_->serve_static("/", "www/index.html", "text/html");
            server_->serve_static("/admin.html", "www/admin.html", "text/html");
            server_->serve_static("/login.html", "www/login.html", "text/html");
            server_->serve_static("/css/style.css", "www/css/style.css", "text/css");
            server_->serve_static("/js/admin.js", "www/js/admin.js", "application/javascript");
            server_->serve_static("/js/login.js", "www/js/login.js", "application/javascript");
            server_->serve_static("/js/public.js", "www/js/public.js", "application/javascript");

            // --- 2. API: AUTENTICACIÓN ---
            server_->post("/api/login", [this](const HttpRequest& req) 
            {
                try 
                {
                    std::string body_str(req.body.begin(), req.body.end());
                    auto j_input = json::parse(body_str);
                    
                    if (j_input.value("password", "") == "admin123") 
                    {
                        return HttpResponse().set_status(HttpStatusCode::OK)
                                             .set_body("{\"token\": \"" + ADMIN_TOKEN + "\"}", "application/json");
                    } 
                    return HttpResponse().set_status(HttpStatusCode::Forbidden).set_body("{\"error\": \"No autorizado\"}", "application/json");
                } 
                catch (...) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest).set_body("{\"error\": \"JSON invalido\"}", "application/json");
                }
            });

            // --- 3. API: LECTURA DE RECURSOS (Sustituye a metadata.json) ---
            server_->get("/api/contenidos", [this](const HttpRequest&) 
            {
                ResultSet rs;
                db_->query("SELECT titulo, autor, tema, filename as file FROM recursos;", {}, rs);

                json db_json = {{"contenidos", json::array()}};
                for (const auto& row : rs) 
                {
                    db_json["contenidos"].push_back({
                        {"titulo", row.at("titulo")},
                        {"autor", row.at("autor")},
                        {"tema", row.at("tema")},
                        {"file", row.at("file")}
                    });
                }
                
                return HttpResponse().set_status(HttpStatusCode::OK).set_body(db_json.dump(), "application/json");
            });

            // --- 4. API: ESCRITURA Y ALMACENAMIENTO FÍSICO ---
            // Nota: Se espera que el cliente envíe los metadatos por URL y el archivo crudo en el body.
            server_->post("/api/upload", [this](const HttpRequest& req) 
            {
                if (req.get_header("Authorization") != ADMIN_TOKEN) 
                {
                    return HttpResponse().set_status(HttpStatusCode::Forbidden).set_body("{\"error\": \"Token invalido\"}", "application/json");
                }

                std::string titulo = req.get_query_param("titulo");
                std::string autor = req.get_query_param("autor");
                std::string tema = req.get_query_param("tema");
                std::string filename = req.get_query_param("filename");

                if (filename.empty() || req.body.empty()) 
                {
                    return HttpResponse().set_status(HttpStatusCode::BadRequest).set_body("{\"error\": \"Faltan datos o archivo vacio\"}", "application/json");
                }

                // 4.1. Escritura en disco (RAII)
                std::string path = UPLOAD_DIR + filename;
                std::ofstream ofs(path, std::ios::binary);
                
                if (!ofs) 
                {
                    return HttpResponse().set_status(HttpStatusCode::InternalServerError).set_body("{\"error\": \"Error de E/S en disco\"}", "application/json");
                }
                
                ofs.write(req.body.data(), req.body.size());
                ofs.close();

                // 4.2. Registro en SQLite
                std::string sql = "INSERT INTO recursos (titulo, autor, tema, filename) VALUES (?, ?, ?, ?);";
                if (db_->execute(sql, {titulo, autor, tema, filename}) == ComponentResult::SUCCESS)
                {
                    return HttpResponse().set_status(HttpStatusCode::Created).set_body("{\"status\": \"success\"}", "application/json");
                }

                return HttpResponse().set_status(HttpStatusCode::InternalServerError).set_body("{\"error\": \"Error en base de datos\"}", "application/json");
            });

            // --- 5. ENTREGA DINÁMICA DE ARCHIVOS (El servidor de la isla) ---
            server_->get("/recursos", [this](const HttpRequest& req) 
            {
                std::string file_requested = req.get_query_param("f"); // ej: /recursos?f=libro.pdf
                if (file_requested.empty()) return HttpResponse().set_status(HttpStatusCode::BadRequest);

                std::string filepath = UPLOAD_DIR + file_requested;
                std::ifstream file(filepath, std::ios::in | std::ios::binary);
                
                if (!file) return HttpResponse().set_status(HttpStatusCode::NotFound);
                
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                return HttpResponse().set_status(HttpStatusCode::OK).set_body(content, "application/octet-stream");
            });
        }

    public:
        Application() = default;
        ~Application() = default;

        void initialize() 
        {
            asegurar_directorio(UPLOAD_DIR);
            
            module_manager_.load_module("./lib/sqlite_handler");
            module_manager_.load_module("./lib/http_server");
            
            db_ = module_manager_.create_instance<IDatabase>("sqlite_handler");
            server_ = module_manager_.create_instance<IHttpServer>("http_server");
            
            if (db_->connect("./data/intraned.db") != ComponentResult::SUCCESS)
            {
                throw std::runtime_error("Fallo critico: No se pudo enlazar SQLite.");
            }

            configure_database();
            configure_routes();
        }

        int run() 
        {
            if (server_->start(8080) != ComponentResult::SUCCESS) 
            {
                return EXIT_FAILURE;
            }

            std::cout << "Intraned operando en Isla Local (http://localhost:8080)" << std::endl;
            std::cin.get(); 

            server_->stop();
            return EXIT_SUCCESS;
        }
};

#endif