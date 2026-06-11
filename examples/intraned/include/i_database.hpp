#ifndef I_DATABASE_HPP
#define I_DATABASE_HPP

#include "i_component.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// Representación genérica de una fila: clave (columna) -> valor (texto)
using DataRow = std::unordered_map<std::string, std::string>;
using ResultSet = std::vector<DataRow>;

class IDatabase : public IComponent 
{
    public:
        virtual ~IDatabase() noexcept = default;

        // Conecta a la base de datos (en el caso de SQLite, abre/crea el archivo)
        virtual ComponentResult connect(const std::string& connection_string) noexcept = 0;
        virtual void disconnect() noexcept = 0;

        /**
         * @brief Ejecuta sentencias que no devuelven datos (INSERT, UPDATE, DELETE).
         * @param query La sentencia SQL con marcadores '?'.
         * @param params Vector con los valores a inyectar de forma segura.
         */
        virtual ComponentResult execute(const std::string& query, const std::vector<std::string>& params) noexcept = 0;

        /**
         * @brief Ejecuta sentencias de lectura (SELECT) y devuelve los datos.
         * @note Se pasa reference wrapper o puntero al ResultSet para mantener noexcept
         */
        virtual ComponentResult query(const std::string& query, const std::vector<std::string>& params, ResultSet& out_result) noexcept = 0;
};

#endif // I_DATABASE_HPP