import { TestBed } from '@angular/core/testing';

import { ImgWorkshopService } from './img-workshop.service';

describe('ImgWorkshopService', () => {
  let service: ImgWorkshopService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(ImgWorkshopService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
