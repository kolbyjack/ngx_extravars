========================    ==========================================================================================
Variable                    Value
------------------------    ------------------------------------------------------------------------------------------
**$aliased_uri**            The part of $uri left over after removing what matches $location.
**$bytes_sent**             Same as `access_log <http://wiki.nginx.org/HttpLogModule#access_log>`_'s $bytes_sent.
**$cache_age**              Age of the cache file used to satisfy requests in seconds.
**$cache_file**             The cache file path for a cached response.
**$cache_key**              The cache key hash for a cached response.
**$connection**             Same as `access_log`_'s $connection.
**$connection_requests**    Number of requests handled by the current connection.
**$ext**                    The extension from $uri.
**$location**               Set to the name of the current location block.
**$msec**                   Same as `access_log`_'s $msec.
**$original_uri**           The original parsed uri.
**$pipe**                   Same as `access_log`_'s $pipe.
**$process_slot**           Slot in nginx's child process list.
**$redirect_count**         The number of times the current request has been internally redirected.
**$request_length**         Same as `access_log`_'s $request_length.
**$request_received**       Timestamp when the request was received, to msec precision.
**$request_time**           Same as `access_log`_'s $request_time.
**$request_version**        HTTP version used by the request.
**$status**                 Same as `access_log`_'s $status.
**$stub_stat_accepted**     Same as the "accepts" field of the stub_status module output.
**$stub_stat_active**       Same as the "active connections" field of the stub_status module output.
**$stub_stat_handled**      Same as the "handled" field of the stub_status module output.
**$stub_stat_reading**      Same as the "reading" field of the stub_status module output.
**$stub_stat_requests**     Same as the "requests" field of the stub_status module output.
**$stub_stat_waiting**      Same as the "waiting" field of the stub_status module output.
**$stub_stat_writing**      Same as the "writing" field of the stub_status module output.
**$subrequest_count**       The number of subrequests performed for this request.
**$time_iso8601**           Same as `access_log`_'s $time_iso8601.
**$time_local**             Same as `access_log`_'s $time_local.
**$wsgi_path_info**         The part of $uri that matches $location, stripped of any trailing /.
**$wsgi_script_name**       The part of $uri that isn't contained in $wsgi_path_info.
========================    ==========================================================================================

