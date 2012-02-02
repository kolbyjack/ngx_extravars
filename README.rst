========================    ==========================================================================================
Variable                    Value
------------------------    ------------------------------------------------------------------------------------------
**$bytes_sent**             Same as `access_log <http://wiki.nginx.org/HttpLogModule#access_log>`_'s $bytes_sent.
**$cache_file**             The cache file path for a cached response.
**$cache_key**              The cache key hash for a cached response.
**$connection**             Same as `access_log`_'s $connection.
**$ext**                    The extension from $uri.
**$location**               Set to the name of the current location block.
**$msec**                   Same as `access_log`_'s $msec.
**$original_uri**           The original parsed uri.
**$pipe**                   Same as `access_log`_'s $pipe.
**$redirect_count**         The number of times the current request has been internally redirected.
**$request_length**         Same as `access_log`_'s $request_length.
**$request_received**       Seconds since the request was received, to msec precision.
**$request_time**           Same as `access_log`_'s $request_time.
**$status**                 Same as `access_log`_'s $status.
**$subrequest_count**       The number of subrequests performed for this request.
**$time_iso8601**           Same as `access_log`_'s $time_iso8601.
**$time_local**             Same as `access_log`_'s $time_local.
========================    ==========================================================================================

