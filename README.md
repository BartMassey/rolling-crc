# Rolling CRC
Copyright (c) 2009 Igor Pavlov  
Copyright (c) 2013 Bulat Ziganshin

This code by Bulat Zighansin *et al* is from this thread
<https://encode.ru/threads/1698-Fast-CRC-table-construction-and-rolling-CRC-hash-calculation>
on the interwebs.

It can currently be made to work when `CRC_INIT_VAL` is 0,
but `BuildRollingCRCTable`, while slow, is also borked.

The code was declared as public domain by the authors.
