
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Viridis palettes in a more useful form for palette lookups,
# and to avoid pulling in the 'viridis' package as a dependency
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
vir <- list()


for (palname in c('A', 'B', 'C', 'D', 'E')) {
  tmp            <- viridis::viridis.map
  vir[[palname]] <- as.matrix(tmp[tmp$opt == palname, 1:3])
  vir[[palname]] <- round(255 * vir[[palname]])
  storage.mode(vir[[palname]]) <- 'integer'
}


vir[['magma'  ]] <- vir[['A']]
vir[['inferno']] <- vir[['B']]
vir[['plasma' ]] <- vir[['C']]
vir[['viridis']] <- vir[['D']]
vir[['cividis']] <- vir[['E']]


usethis::use_data(vir, internal = TRUE, compress = 'xz', overwrite = TRUE)


# pal <- vir$D
#
# saveRDS(pal, file = "~/pal.rds")
