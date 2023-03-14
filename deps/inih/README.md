# inih Parser Library

Originally from https://github.com/benhoyt/inih.

Code is (c)2009 Ben Hoyt, under 3-clause BSD license, see [LICENSE.txt](./LICENSE.txt).

Reproduced here so crinit-specific compile-time options can be set for the parser.

We deviate from inih's defaults in the following way:
* `INI_MAX_LINE=4096` to allow very long values if needed.
* `INI_ALLOW_INLINE_COMMENTS=0` to disallow inline comments which interact badly with quoting

