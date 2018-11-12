from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import bisect
import hashlib
import heapq
import mmap
import os
import sys
import tempfile
import threading
import time
import types
import uuid

from time import sleep

try:
    from http.client import CannotSendRequest, ResponseNotReady
    import http.client as httplib
except BaseException:
    from httplib import CannotSendRequest, ResponseNotReady
    import httplib

try:
    from urllib.parse import urlparse
except BaseException:
    from urlparse import urlparse

# Import SQLite
try:
    sys.path.append(os.path.dirname(__file__) + '/third_party/')
    sys.path.append(os.path.dirname(__file__) + '/third_party/internal/')
    from supersqlite.third_party.internal.pysqlite2 import dbapi2 as sqlite3
    db = sqlite3.connect(':memory:')
    db.close()
    SQLITE_LIB = 'internal'
except Exception:
    import sqlite3
    SQLITE_LIB = 'system'

# Import SQLite (APSW)
try:
    import supersqlite.third_party.internal.apsw as apsw
    db = apsw.Connection(':memory:')
    db.close()
    APSW_LIB = 'internal'
except Exception as e:
    raise(e)
    APSW_LIB = 'none'

# Exportable modules
pysqlite = sqlite3
apsw = apsw


class SuperSQLite():
    pass


class SuperSQLiteConnection(apsw.Connection):
    pass


for prop in dir(apsw):
    value = getattr(apsw, prop)
    if prop.startswith('__') or isinstance(value, types.ModuleType):
        continue
    setattr(SuperSQLite, prop, value)
setattr(SuperSQLite, 'connect', SuperSQLiteConnection)

# KeyList helper class


class KeyList(object):
    def __init__(self, ls, key):
        self.ls = ls
        self.key = key

    def __len__(self):
        return len(self.ls)

    def __getitem__(self, index):
        return self.key(self.ls[index])


if APSW_LIB == 'internal':

    class HTTPVFSFileCache():
        """ This cache sort of acts like a predictor for sequential
            network reads. It proactively pulls in more data than
            requested from the network if it sees a pattern of sequential
            reads. The amount of data predictively pulled is
            adjusts based on the last few true sequential reads.
        """

        def __init__(self, vfsfile):
            self.vfsfile = vfsfile
            self.cache_size = None
            self._start_offset = 0
            self.running_hit_direction = 0
            self.running_hit_last_start = float("inf")
            self.running_hit_last_end = 0
            self.running_forward_hit_amount = 0
            self.running_backward_hit_amount = 0
            self.running_hit_amount = 0
            self.time = time.time()
            self.id = uuid.uuid4().int
            self.data = "".encode('utf-8')

        def length_of_data(self):
            """Returns the length of the cached data."""
            return len(self.data)

        def get_data(self):
            """Returns the cached data."""
            return self.data

        def set_data(self, data):
            """Sets the cached data."""
            self.data = data

        def add_to_caches(self):
            """Adds self to the caches."""
            self.vfsfile.caches.append(self)

        def save_cache(self):
            """Saves the cache."""
            pass

        def delete_caches(self):
            """Deletes old caches."""
            current_time = time.time()
            self.vfsfile.caches = [
                cache for cache in self.vfsfile._get_caches() if (
                    current_time - cache.time) <= self.vfsfile.cache_ttl]

        def get_cache(self, amount, offset):
            """Checks if a cache exists for the data offset, and amount to read,
               if so, return the cache, and the start and end range to read
               from the cache's data.

               Keeps track of forward sequential reads, and backward
               sequential reads for the cache.
            """
            return_val = [None, None, None, None, None, None, None]
            measure_cache_size = self.cache_size is None
            if measure_cache_size:
                self.cache_size = 0
            for c in self.vfsfile._get_caches():
                if measure_cache_size:
                    self.cache_size += c.length_of_data()
                start = offset - c._start_offset
                end = start + amount
                close_to_last_end = (
                    abs(start - c.running_hit_last_end) <
                    self.vfsfile.sequential_cache_gap_tolerance)
                close_to_last_start = (
                    abs(c.running_hit_last_start - end) <
                    self.vfsfile.sequential_cache_gap_tolerance)
                small_read = self.vfsfile.sequential_cache_default_read * 2  # noqa
                if start >= 0 and c.length_of_data() >= end:
                    # Cache hit
                    # Keeps track of the total running
                    # amount of sequentially read
                    # bytes on the cache, and the direction
                    if start >= c.running_hit_last_end:
                        # Forward sequential
                        c.running_forward_hit_amount = \
                            c.running_forward_hit_amount + amount
                        if (c.running_forward_hit_amount !=
                                c.running_backward_hit_amount):
                            c.running_hit_direction = max(
                                (c.running_forward_hit_amount, 1),
                                (c.running_backward_hit_amount, -1))[1]
                        else:
                            c.running_hit_direction = 1
                    if end <= c.running_hit_last_start:
                        # Backward sequential
                        c.running_backward_hit_amount = \
                            c.running_backward_hit_amount + amount
                        if (c.running_forward_hit_amount !=
                                c.running_backward_hit_amount):
                            c.running_hit_direction = max(
                                (c.running_forward_hit_amount, 1),
                                (c.running_backward_hit_amount, -1))[1]
                        else:
                            c.running_hit_direction = -1
                    c.running_hit_amount = max(
                        c.running_forward_hit_amount,
                        c.running_backward_hit_amount)
                    c.running_hit_last_start = start
                    c.running_hit_last_end = end
                    c.time = time.time()
                    return_val = (
                        c.running_hit_amount,
                        c.running_hit_direction,
                        c.running_forward_hit_amount,
                        c.running_backward_hit_amount,
                        start,
                        end,
                        c
                    )
                    c.save_cache()
                elif (
                    (return_val[0] is None or (isinstance(return_val, list) and
                                               c.running_hit_amount > return_val[0])) and  # noqa
                    start >= c.running_hit_last_end and
                    close_to_last_end
                ):
                    # Complete cache miss, but it is still a close forward
                    # sequential read of the current cache, return
                    # the running sequentially read byte information
                    # so it can be added to the next cache
                    return_val[1] = 1
                    if return_val[1] != c.running_hit_direction:
                        return_val[0] = small_read
                        return_val[2] = small_read
                        return_val[3] = small_read
                    else:
                        return_val[0] = c.running_hit_amount
                        return_val[2] = c.running_forward_hit_amount
                        return_val[3] = c.running_backward_hit_amount
                elif (
                    (return_val[0] is None or (isinstance(return_val, list) and
                                               c.running_hit_amount > return_val[0])) and  # noqa
                    end <= c.running_hit_last_start and
                    close_to_last_start
                ):
                    # Partial cache miss, but it is still a close backward
                    # sequential read of the current cache, return
                    # the running sequentially read byte information
                    # so it can be added to the next cache
                    return_val[1] = -1
                    if return_val[1] != c.running_hit_direction:
                        return_val[0] = small_read
                        return_val[2] = small_read
                        return_val[3] = small_read
                    else:
                        return_val[0] = c.running_hit_amount
                        return_val[2] = c.running_forward_hit_amount
                        return_val[3] = c.running_backward_hit_amount
            return return_val

        def write_data(self, start_offset, data, amount, offset):
            """Writes data fetched to the network cache and
            returns only the amount requested back."""
            # Writes the entire data fetched to the cache
            if self.vfsfile.should_cache:
                # Uses itself as a cache object
                self._start_offset = start_offset
                self.set_data(data)
                if self.vfsfile.trace_log:
                    print("[HTTPVFS] Cache Size: %d bytes" % (self.cache_size,))

                # Purge old caches
                current_time = time.time()
                if ((current_time -
                        self.vfsfile.last_cache_purge) >
                        self.vfsfile.ttl_purge_interval):
                    if self.vfsfile.trace_log:
                        print("[HTTPVFS] Purging expired caches...")
                    self.vfsfile.last_cache_purge = current_time
                    self.delete_caches()

                # Adds itself to the cache array, so the next read
                # succeed
                self.add_to_caches()

            return data[offset -
                        start_offset: (offset - start_offset) + amount]

        def _prefetch_in_background(
                self,
                _prefetch_in_background,
                amount,
                offset,
                sequential):
            """Prefetches data from the network to the cache."""
            # Store the extra data fetched back in the network cache
            if self.vfsfile.trace_log:
                print(
                    "[HTTPVFS] Prefetching in background @ %d + %d" %
                    (offset, amount))
            try:
                if sequential:
                    data = _prefetch_in_background(
                        self.vfsfile.SEQUENTIAL, amount, offset)
                else:
                    data = _prefetch_in_background(
                        self.vfsfile.RANDOM_ACCESS, amount, offset)
                cache = HTTPVFSFileCache(self.vfsfile)
                if data:
                    cache.write_data(offset, data, 0, offset)
                    if self.vfsfile.trace_log:
                        print(
                            "[HTTPVFS] Finished prefetching @ %d + %d" %
                            (offset, amount))
                else:
                    if self.vfsfile.trace_log:
                        print(
                            "[HTTPVFS] Prefetching terminated early @ %d + %d" %
                            (offset, amount))
            except BaseException:
                if self.vfsfile.trace_log:
                    print(
                        "[HTTPVFS] Prefetching error @ %d + %d" %
                        (offset, amount))
                pass

        def prefetch_in_background(
                self,
                _prefetch_in_background,
                amount,
                offset,
                sequential=False):
            """Prefetches data from the network to the cache
            in the background."""
            if self.vfsfile.trace_log:
                if sequential:
                    print(
                        "[HTTPVFS] Sequential prefetching "
                        "request @ %d + %d" %
                        (offset, amount))
                else:
                    print(
                        "[HTTPVFS] Random access prefetching "
                        "request @ %d + %d" %
                        (offset, amount))
            self.vfsfile.prefetch_threads = [
                t for t in self.vfsfile.prefetch_threads if t.is_alive()]
            if (len(self.vfsfile.prefetch_threads) <=
                    self.vfsfile.prefetch_thread_limit or sequential):
                prefetch_thread = threading.Thread(
                    target=self._prefetch_in_background,
                    args=(
                        _prefetch_in_background,
                        amount,
                        offset,
                        sequential),
                    name='HTTPVFSFileCache' +
                    (
                        'Sequential' if sequential else '') +
                    'PrefetchThread@' +
                    str(offset) +
                    '+' +
                    str(amount))
                prefetch_thread.daemon = True
                if sequential:
                    if self.vfsfile.sequential_prefetch_thread:
                        self.vfsfile.sequential_prefetch_thread.do_run = False
                    self.vfsfile.sequential_prefetch_thread = prefetch_thread
                else:
                    self.vfsfile.prefetch_threads.append(prefetch_thread)
                prefetch_thread.start()
            else:
                if self.vfsfile.trace_log:
                    print(
                        "[HTTPVFS] Ignoring prefetch request @ %d + %d, "
                        "reached prefetch thread limit" %
                        (offset, amount))

        def read_data(self, amount, offset, _prefetch_in_background=None):
            """Reads data from the network cache and
            returns only the amount requested back or
            Returns None if there is a cache miss, and prefetches more data
            into the cache using _prefetch_in_background(amount, offset)
            if it detects a non-sequential access pattern in the
            cache misses."""

            # Don't do anything if caching is disabled
            if not self.vfsfile.should_cache:
                return None

            # Find the closest cache match
            current_time = time.time()
            (
                running_hit_amount,
                running_hit_direction,
                running_forward_hit_amount,
                running_backward_hit_amount,
                start,
                end,
                cache
            ) = self.get_cache(amount, offset)
            if running_hit_amount is not None:
                if (self.vfsfile.sequential_cache_exponential_read_growth and
                        cache is None):
                    # Reached a cache miss, but still sequentially reading
                    # If exponential sequential cache reads are on, double the
                    # read size
                    running_hit_amount = min(
                        running_hit_amount * 2,
                        self.vfsfile.sequential_cache_max_read)
                    running_forward_hit_amount = min(
                        running_forward_hit_amount * 2,
                        self.vfsfile.sequential_cache_max_read)
                    running_backward_hit_amount = min(
                        running_backward_hit_amount * 2,
                        self.vfsfile.sequential_cache_max_read)
                self.running_forward_hit_amount = running_forward_hit_amount
                self.running_backward_hit_amount = running_backward_hit_amount
                self.running_hit_amount = running_hit_amount
                self.running_hit_direction = running_hit_direction
                self.vfsfile.running_hit_direction = running_hit_direction
                if cache is None:
                    self.vfsfile.cache_amount = min(
                        running_hit_amount,
                        self.vfsfile.sequential_cache_max_read
                    )
                    self.save_cache()
            else:
                if cache is None:
                    # Cache miss, and not a sequential read, only read a small
                    self.vfsfile.cache_amount = \
                        self.vfsfile.sequential_cache_default_read
                    self.save_cache()

            if cache:
                data = cache.get_data()[start:end]

                # Adjust the cache amount for the next read
                self.vfsfile.running_hit_direction = cache.running_hit_direction
                self.vfsfile.cache_amount = min(
                    cache.running_hit_amount,
                    self.vfsfile.sequential_cache_max_read)

                return data
            elif self.vfsfile.random_access_cache_prefetch:
                # Keep track of regions of the file where there are cache
                # misses. Each "hit" on a file is analyzed and clustered into
                # "groups" of hits, sequential "hits" are ignored.

                # Purge old hit patterns
                if (current_time - self.vfsfile.last_random_access_hit_tracker_purge) > self.vfsfile.ttl_purge_interval:  # noqa
                    if self.vfsfile.trace_log:
                        print("[HTTPVFS] Purging expired hit trackers...")
                    self.vfsfile.last_random_access_hit_tracker_purge = \
                        current_time
                    self.vfsfile.hit_pattern = [hit for hit in self.vfsfile.hit_pattern if ((current_time - hit[4]) <= self.vfsfile.random_access_hit_tracker_ttl)]  # noqa

                # Find the closest cluster of hits for the current miss
                hit_index = bisect.bisect_left(
                    KeyList(
                        self.vfsfile.hit_pattern,
                        key=lambda x: x[0]),
                    offset)
                hit_index_area = []
                if hit_index - 1 >= 0:
                    hit_index_area.append(hit_index - 1)
                if hit_index < len(self.vfsfile.hit_pattern):
                    hit_index_area.append(hit_index)
                if len(hit_index_area) > 0:
                    hit_index = min(
                        hit_index_area, key=lambda x: abs(
                            self.vfsfile.hit_pattern[x][0] - offset))

                    # Add the current miss to the closest cluster, and evaluate
                    # if it should be prefetched
                    hit = self.vfsfile.hit_pattern[hit_index]
                    dist = abs(hit[0] - offset)
                    if dist <= self.vfsfile.random_access_cache_range:
                        self.vfsfile.hit_pattern[hit_index] = [
                            (offset + hit[0]) / 2.0,
                            (dist + hit[1]) / 2.0 if dist > hit[1] else hit[1],
                            hit[2] + 1 if offset > hit[0] else hit[2],
                            hit[3] + 1 if offset < hit[0] else hit[3],
                            current_time]
                        hit = self.vfsfile.hit_pattern[hit_index]
                        if hit[2] >= hit[3] * 2 and (hit[2] + hit[3]) > 8:
                            # Looks like a forward sequential read pattern,
                            # ignore
                            del self.vfsfile.hit_pattern[hit_index]
                        elif hit[3] >= hit[2] * 2 and (hit[2] + hit[3]) > 8:
                            # Looks like a backward sequential read pattern,
                            # ignore
                            del self.vfsfile.hit_pattern[hit_index]
                        elif (_prefetch_in_background and (hit[2] > 2) and
                              (hit[3] > 2) and (hit[2] + hit[3]) > 30):
                            # If a certain region of the file, is being "hit"
                            # frequently for smaall chunks of data within a
                            # larger range, prefetch that region of the file
                            # and data surrounding it to prevent future
                            # cache misses
                            self.prefetch_in_background(
                                _prefetch_in_background, int(
                                    hit[1] * 2), max(int(hit[0] - hit[1]), 0)
                            )

                        return None
                # mean, range, positive direction, negative direction, time
                self.vfsfile.hit_pattern.insert(
                    hit_index, [offset, 0, 0, 0, current_time])

    class HTTPVFSFileMemoryMappedCache(HTTPVFSFileCache):
        """ This cache is like HTTPVFSFileCache
            except all cache data is memory mapped
        """

        def __init__(self, vfsfile, cache_dir_path, cache_key=None):
            self.cache_dir_path = cache_dir_path
            self.cache_key = cache_key
            HTTPVFSFileCache.__init__(self, vfsfile)
            if self.cache_key and self.cache_key != '.DS_Store':
                cache_key_split = cache_key.split('.')[0].split('_')
                self._start_offset = int(cache_key_split[0])
                self.running_hit_direction = int(cache_key_split[1])
                self.running_hit_last_start = (
                    float(
                        cache_key_split[2])
                    if cache_key_split[2] == 'inf' else int(
                        cache_key_split[2]))
                self.running_hit_last_end = int(cache_key_split[3])
                self.running_forward_hit_amount = int(cache_key_split[4])
                self.running_backward_hit_amount = int(cache_key_split[5])
                self.running_hit_amount = int(cache_key_split[6])
                self.time = float(cache_key_split[7])
                self.id = int(cache_key_split[8])
            else:
                self.cache_key = self.create_key()

        def length_of_data(self):
            """Returns the length of the cached data."""
            try:
                return os.path.getsize(os.path.join(self.cache_dir_path,
                                                    self.cache_key))
            except BaseException:
                return 0

        def add_to_mmaps(self, new, mm):
            """Adds a new mmap, evicting old mmaps if the maximum has been
            reached."""
            while (len(self.vfsfile.cache_mmaps_heap) >=
                   self.vfsfile.mmap_max_files):
                _, evict = heapq.heappop(self.vfsfile.cache_mmaps_heap)
                try:
                    evict_mm = self.vfsfile.cache_mmaps[evict]
                except BaseException:
                    pass
                try:
                    evict_mm.close()
                except BaseException:
                    pass
                try:
                    del self.vfsfile.cache_mmaps[evict]
                except BaseException:
                    pass
            heapq.heappush(self.vfsfile.cache_mmaps_heap,
                           (time.time(), new))
            self.vfsfile.cache_mmaps[new] = mm

        def get_mmap(self, create=True):
            """Gets the mmap for a key, opening a mmap to the file
            if a mmap doesn't exist, creating a file, then opening a mmap
            to it if the file doesn't exist."""
            if (self.cache_key not in self.vfsfile.cache_mmaps and create):
                joined = os.path.join(self.cache_dir_path,
                                      self.cache_key)
                if os.path.exists(os.path.join(self.cache_dir_path,
                                               self.cache_key)):
                    f = open(joined, "r+b")
                    mm = mmap.mmap(f.fileno(), self.length_of_data())
                    f.close()
                else:
                    f = open(joined, "w+b")
                    f.write("\0".encode('utf-8'))
                    f.flush()
                    os.fsync(f.fileno())
                    mm = mmap.mmap(f.fileno(), 1)
                    f.close()
                self.add_to_mmaps(self.cache_key, mm)
            try:
                return self.vfsfile.cache_mmaps[self.cache_key]
            except BaseException as e:
                if create:
                    return e
                else:
                    return None

        def get_data(self):
            """Returns the cached data."""
            return self.get_mmap()

        def set_data(self, data):
            """Sets the cached data."""
            self.save_cache()
            mm = self.get_mmap(create=False)
            try:
                del self.vfsfile.cache_mmaps[self.cache_key]
            except BaseException:
                pass
            try:
                mm.close()
            except BaseException:
                pass
            f = open(os.path.join(self.cache_dir_path,
                                  self.cache_key), "w+b")
            f.write(data)
            f.flush()
            os.fsync(f.fileno())
            mm = None
            mm = mmap.mmap(f.fileno(), len(data))
            f.close()
            self.vfsfile.cache_mmaps[self.cache_key] = mm

        def create_key(self):
            """Serializes instance variables into a key."""
            return '_'.join([
                str(self._start_offset),
                str(self.running_hit_direction),
                str(self.running_hit_last_start),
                str(self.running_hit_last_end),
                str(self.running_forward_hit_amount),
                str(self.running_backward_hit_amount),
                str(self.running_hit_amount),
                str(int(self.time)),
                str(self.id),
            ]) + '.supersqlmmap'

        def add_to_caches(self):
            """Adds self to the caches."""
            pass

        def save_cache(self):
            """Saves the cache."""
            new_key = self.create_key()
            old = os.path.join(self.cache_dir_path,
                               self.cache_key)
            new = os.path.join(self.cache_dir_path, new_key)
            try:
                os.rename(old, new)
            except BaseException:
                pass
            try:
                mm = self.vfsfile.cache_mmaps[self.cache_key]
                del self.vfsfile.cache_mmaps[self.cache_key]
                self.add_to_mmaps(new_key, mm)
            except BaseException:
                pass
            self.cache_key = new_key

        def delete_caches(self):
            """Deletes old caches."""
            current_time = time.time()
            for cache in self.vfsfile._get_caches():
                if cache.id == self.id:
                    continue
                if (current_time - cache.time) > self.vfsfile.cache_ttl:
                    try:
                        mmap = cache.get_mmap(create=False)
                    except BaseException:
                        pass
                    try:
                        del self.vfsfile.cache_mmaps[self.cache_key]
                    except BaseException:
                        pass
                    try:
                        mmap.close()
                    except BaseException:
                        pass
                    try:
                        os.remove(os.path.join(cache.cache_dir_path,
                                               cache.cache_key))
                    except BaseException:
                        pass

    class HTTPVFSFile(apsw.VFSFile):
        """ This acts as the representation of a single file on
            the HTTP virtual file system.
        """

        def __init__(self, inheritfromvfsname, name, flags, vfs, options=None):
            # Constants
            self.RANDOM_ACCESS = 0
            self.SEQUENTIAL = 1

            # Cache + Network configuration
            defaults = {
                'should_cache': True,
                'network_retry_delay': 10,
                'max_network_retries': 10,
                'sequential_cache_default_read': 4096 * 2,
                'sequential_cache_gap_tolerance': 10 * (1024 ** 2),
                'sequential_cache_max_read': 20 * (1024 ** 2),
                'sequential_cache_exponential_read_growth': True,
                'prefetch_thread_limit': 3,
                'sequential_cache_prefetch': True,
                'random_access_cache_prefetch': True,
                'random_access_cache_range': 100 * (1024 ** 2),
                'random_access_hit_tracker_ttl': 60,
                'cache_ttl': 60,
                'ttl_purge_interval': 5,
                'use_mmap': False,
                'mmap_max_files': 10,
                'temp_dir': tempfile.gettempdir(),
                'trace_log': False,
            }
            defaults.update(options or {})
            for k, v in defaults.items():
                setattr(self, k, v)
            self.max_network_retries = max(self.max_network_retries, 4)
            if not self.should_cache:
                self.sequential_cache_prefetch = False
                self.random_access_cache_prefetch = False
                self.sequential_cache_default_read = 0
                self.cache_amount = 0

            # Cache initialization
            self.caches = []
            self.cache_mmaps_heap = []
            self.cache_mmaps = {}
            self.cache_amount = self.sequential_cache_default_read
            self.last_cache_purge = 0
            self.last_random_access_hit_tracker_purge = 0

            # Prefetch Connections
            self.pconn_terminated = {}
            self.pconn_count = {}
            self.pconn = {}

            # Connection lock
            self.conn_lock = threading.RLock()

            # State to keep tracking adjusting the predictive network cache
            # window
            self.running_hit_direction = 0
            self.hit_pattern = []

            # Keep track of threads
            self.prefetch_threads = []
            self.sequential_prefetch_thread = None

            # Initialization
            self.vfs = vfs
            self.length = 99999999999999999
            self.name = name
            self.tries = 1
            self.url = self.name.filename()
            url_cis = self.url.lower()
            try:
                self.url = self.url[url_cis.index('http://'):]
                self.parsed_url = urlparse(self.url)
                self._prepare_connection()
                if self.random_access_cache_prefetch:
                    self._prepare_prefetch_connection(self.RANDOM_ACCESS)
                if self.sequential_cache_prefetch:
                    self._prepare_prefetch_connection(self.SEQUENTIAL)
            except BaseException:
                try:
                    self.url = self.url[url_cis.index('https://'):]
                    self.parsed_url = urlparse(self.url)
                    self._prepare_connection()
                    if self.random_access_cache_prefetch:
                        self._prepare_prefetch_connection(self.RANDOM_ACCESS)
                    if self.sequential_cache_prefetch:
                        self._prepare_prefetch_connection(self.SEQUENTIAL)
                except BaseException:
                    raise RuntimeError("Invalid URL.")
            self.cache_dir = (
                hashlib.md5(
                    self.url.encode('utf-8')).hexdigest() +
                '_supersqlmmap')
            self.cache_dir_path = os.path.join(self.temp_dir, self.cache_dir)
            try:
                os.makedirs(self.cache_dir_path + '/')
            except OSError:
                pass

            # Prepare the VFS
            apsw.VFSFile.__init__(self, inheritfromvfsname, os.devnull, flags)

        def _new_connection(self):
            """Creates an HTTP connection"""
            if self.parsed_url.scheme.lower() == 'http':
                return httplib.HTTPConnection(
                    self.parsed_url.netloc, timeout=60)
            else:
                return httplib.HTTPSConnection(
                    self.parsed_url.netloc, timeout=60)

        def _prepare_connection(self, new=True):
            """Prepares a new HTTP connection"""
            try:
                self.conn.close()
            except BaseException:
                pass
            if new:
                self.conn = self._new_connection()

        def _prepare_prefetch_connection(self, n, new=True):
            """Prepares a new HTTP connection"""
            try:
                self.pconn_terminated[n] = True
                while self.pconn_count[n] > 0:
                    sleep(1)
                self.pconn[n].close()
            except BaseException:
                pass
            if new:
                self.pconn[n] = self._new_connection()
                self.pconn_count[n] = 0
                self.pconn_terminated[n] = False

        def _wait_on_prefetch_connection(self, n):
            self.pconn_count[n] += 1

        def _unwait_on_prefetch_connection(self, n):
            self.pconn_count[n] -= 1

        def _network_error(self, e, i):
            """Handles an network error"""
            if self.trace_log:
                print("[HTTPVFS] Network Error: %s" % (str(e),))
            if i + 1 >= self.tries:
                raise RuntimeError(
                    "Could not reach the server at: '" + self.url + "'")
            else:
                if self.trace_log:
                    print("[HTTPVFS] Refreshing network connection...")
                self.conn_lock.acquire()
                self._prepare_connection()
                self.conn_lock.release()
                if i > 2:
                    if self.trace_log:
                        print("[HTTPVFS] Waiting before retrying...")
                    sleep(self.network_retry_delay)
                    if self.trace_log:
                        print("[HTTPVFS] Retrying...")

        def _prefetch_in_background(self, n, amount, offset):
            headers = {
                'Range': "bytes=" + str(max(offset, 0)) + "-" + str(
                    min((offset + amount) - 1, self.length)  # noqa
                ),
            }

            self._wait_on_prefetch_connection(n)
            while not self.pconn_terminated[n]:
                try:
                    self.pconn[n].request(
                        "GET", self.parsed_url.path, headers=headers)
                    break
                except CannotSendRequest:
                    sleep(1)
            while not self.pconn_terminated[n]:
                try:
                    res = self.pconn[n].getresponse()
                    break
                except ResponseNotReady:
                    # Since we are sharing the connection wait for this to be
                    # ready
                    sleep(1)
            if self.pconn_terminated[n]:
                self._unwait_on_prefetch_connection(n)
                return
            else:
                self._unwait_on_prefetch_connection(n)

            if not(res.status >= 200 and res.status <= 299):
                # Check for a valid status from the server
                return
            data = bytearray(res.length)
            i = 0
            for piece in iter(lambda: res.read(1024), bytes('')):
                if not getattr(threading.currentThread(), "do_run", True):
                    break
                data[i:i + len(piece)] = piece
                i = i + len(piece)
            else:
                return bytes(data)

            # Leaving the thread early, without
            # reading all of the data this will
            # make the connection unusable, refresh it
            self._prepare_prefetch_connection(n)

        def _get_caches(self):
            """Gets all of the caches."""
            if self.use_mmap:
                return [
                    HTTPVFSFileMemoryMappedCache(
                        self,
                        self.cache_dir_path,
                        cache_key) for cache_key in os.listdir(
                        self.cache_dir_path)]

            else:
                return self.caches

        def xRead(self, amount, offset):  # noqa: N802
            """Intercepts SQLite's file read command"""
            if self.trace_log:
                print("[HTTPVFS] Read request @ %d + %d" % (offset, amount))
            for i in range(self.tries):
                try:
                    # Try to see if we have already read the data
                    # and cached it
                    if self.use_mmap:
                        cache = HTTPVFSFileMemoryMappedCache(
                            self, self.cache_dir_path)
                    else:
                        cache = HTTPVFSFileCache(self)
                    data = cache.read_data(
                        amount, offset, self._prefetch_in_background)
                    if data is None:
                        if self.trace_log and self.should_cache:
                            print(
                                "[HTTPVFS] Cache miss for request @ %d + %d" %
                                (offset, amount))

                        # Fire off a network request with the range of bytes
                        # (potentially predicatively reading a larger amount
                        # and storing it in the network cache)
                        if self.running_hit_direction >= 0:
                            # Read the amount requested + extra
                            # in the forward sequential direction
                            # to save in the cache
                            start = max(offset, 0)
                            end = min(
                                (offset + max(self.cache_amount, amount)) - 1,
                                self.length)
                        else:
                            # Read the amount requested + extra
                            # in the backward sequential direction
                            # to save in the cache
                            start = max(offset - self.cache_amount, 0)
                            end = min((offset + amount) - 1, self.length)

                        # Cancel any previous sequential prefetches, the current
                        # chunk data of data was requested too fast for any
                        # background prefetches to load the cache, must
                        # request it synchronously
                        if self.sequential_prefetch_thread:
                            self.sequential_prefetch_thread.do_run = False

                        # Synchronously request the current chunk from the
                        # network
                        headers = {
                            'Range': "bytes=" + str(start) + "-" + str(end),
                        }
                        self.conn_lock.acquire()
                        self.conn.request(
                            "GET", self.parsed_url.path, headers=headers)
                        res = self.conn.getresponse()
                        if not(res.status >= 200 and res.status <= 299):
                            # Check for a valid status from the server
                            raise RuntimeError(
                                "HTTP Status Code Error from Server")
                        if self.trace_log:
                            print(
                                "[HTTPVFS] Fetching @ %d + %d for "
                                "request @ %d + %d" %
                                (start, 1 + end - start, offset, amount))
                        data = res.read()
                        self.conn_lock.release()
                        if self.trace_log:
                            print(
                                "[HTTPVFS] Done fetching @ %d + %d for "
                                "request @ %d + %d" %
                                (start, 1 + end - start, offset, amount))

                        # Store the extra data fetched back in the network cache
                        data = cache.write_data(start, data, amount, offset)

                        # Prefetch the next sequential chunk of data in the
                        # background
                        if self.sequential_cache_prefetch and self.should_cache:
                            if self.running_hit_direction >= 0:
                                cache.prefetch_in_background(
                                    self._prefetch_in_background,
                                    self.cache_amount,
                                    start + self.cache_amount * 1,
                                    sequential=True)
                            else:
                                cache.prefetch_in_background(
                                    self._prefetch_in_background,
                                    self.cache_amount,
                                    start - self.cache_amount * 1,
                                    sequential=True)
                    else:
                        if self.trace_log:
                            print(
                                "[HTTPVFS] Cache hit for request @ %d + %d" %
                                (offset, amount))

                    # Return the data to SQLite
                    return data
                except BaseException as e:
                    try:
                        self.conn_lock.release()
                    except BaseException:
                        pass
                    # Handle a network error
                    self._network_error(e, i)

        def xWrite(self, data, offset):  # noqa: N802
            """Intercepts SQLite's file write command"""
            # Can't write to an HTTP server, ignore
            pass

        def xFileSize(self):  # noqa: N802
            """Intercepts SQLite's file size command"""
            for i in range(self.tries):
                try:
                    # Fire of a content-length request to the server
                    self.conn_lock.acquire()
                    self.conn.request("GET", self.parsed_url.path)
                    res = self.conn.getresponse()
                    self.tries = self.max_network_retries
                    self.length = res.length
                    self._prepare_connection()
                    self.conn_lock.release()
                    return self.length
                except BaseException as e:
                    try:
                        self.conn_lock.release()
                    except BaseException:
                        pass
                    # Handle a network error
                    self._network_error(e, i)

        def xClose(self):  # noqa: N802
            """Intercepts SQLite's file close command"""
            ident = self.name.filename()
            with self.vfs.files_lock:
                if ident in self.vfs.files:
                    if self.vfs.files[ident][0] <= 1:
                        for t in self.prefetch_threads:
                            t.do_run = False
                        if self.sequential_prefetch_thread:
                            self.sequential_prefetch_thread.do_run = False
                        self._prepare_prefetch_connection(
                            self.RANDOM_ACCESS, new=False)
                        self._prepare_prefetch_connection(
                            self.SEQUENTIAL, new=False)
                        self._prepare_connection(new=False)
                        del self.vfs.files[ident]
                        while len(self.cache_mmaps_heap) >= 0:
                            _, evict = heapq.heappop(self.cache_mmaps_heap)
                            try:
                                evict_mm = self.cache_mmaps[evict]
                            except BaseException:
                                pass
                            try:
                                evict_mm.close()
                            except BaseException:
                                pass
                            try:
                                del self.cache_mmaps[evict]
                            except BaseException:
                                pass
                    else:
                        self.vfs.files[ident] = (
                            self.vfs.files[ident][0] - 1,
                            self.vfs.files[ident][1])

    class HTTPVFS(apsw.VFS):
        """ This acts as the representation of a filesystem that
            proxies to HTTP requests so that SQLite can connect
            to HTTP URLs.
        """

        def __init__(self, vfsname="http", basevfs="", options=None):
            self.vfsname = vfsname
            self.basevfs = basevfs
            self.options = options or {}
            apsw.VFS.__init__(self, self.vfsname, self.basevfs)
            self.files = {}
            self.files_lock = threading.RLock()

        def xOpen(self, name, flags=apsw.SQLITE_OPEN_MAIN_DB):  # noqa: N802
            """Intercepts SQLite's file open command"""
            flags[1] = flags[1] | apsw.SQLITE_OPEN_READONLY
            if flags[0] & apsw.SQLITE_OPEN_MAIN_DB:
                ident = name.filename()
                with self.files_lock:
                    if ident not in self.files:
                        self.files[ident] = (1, HTTPVFSFile(
                            self.basevfs, name, flags, self, self.options))
                    else:
                        self.files[ident] = (
                            self.files[ident][0] + 1, self.files[ident][1])
                    return self.files[ident][1]
            else:
                return None
