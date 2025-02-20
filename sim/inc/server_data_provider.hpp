#ifndef NOS3_SERVERDATAPROVIDER_HPP
#define NOS3_SERVERDATAPROVIDER_HPP

#include <boost/property_tree/xml_parser.hpp>
#include <ItcLogger/Logger.hpp>
#include <server_data_point.hpp>
#include <sim_i_data_provider.hpp>

namespace Nos3
{
    class ServerDataProvider : public SimIDataProvider
    {
    public:
        /* Constructors */
        ServerDataProvider(const boost::property_tree::ptree& config);

        /* Accessors */
        boost::shared_ptr<SimIDataPoint> get_data_point(void) const;

    private:
        /* Disallow these */
        ~ServerDataProvider(void) {};
        ServerDataProvider& operator=(const ServerDataProvider&) {return *this;};

        mutable double _request_count;
    };
}

#endif
