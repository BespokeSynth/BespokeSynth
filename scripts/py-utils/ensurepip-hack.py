# Ensurepip on debian/ubuntu disables ensurepip but we need it so

import ensurepip
ensurepip._ensurepip_is_disabled_in_debian_for_system = lambda: True
ensurepip._main()
